#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "solar.h"

uint16_t writeSuccess = 0;

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);


uint16_t getWriteSuccessInfluxDB() {
  return writeSuccess;
}

bool updateClock( bool firstStart, const char* tzInfo, const char* ntp) {
  //Convert ntp comma-separated list to array
  char ntpBuff[50];
  char *ntpArr[3];
  strncpy(ntpBuff, ntp, sizeof( ntpBuff));
  ntpArr[0] = strtok( ntpBuff, ",");
  for (unsigned int i = 1; i < sizeof( ntpArr)/sizeof(char*); i++)
    ntpArr[i] = strtok( NULL, ",");

  //Set TZ and NTP
  configTzTime(tzInfo,ntpArr[0], ntpArr[1], ntpArr[2]);
  if (firstStart) {
    // Wait till time is synced
    Serial.print(F("Syncing time"));
    int i = 0;
    while (time(nullptr) < 1000000000ul && i < 40) {
      Serial.print(F("."));
      delay(500);
      i++;
    }
    Serial.println();

    // Show time
    time_t now = time(nullptr);
    Serial.print(F("Synchronized time: "));
    Serial.println(ctime(&now));
  }
  return time(nullptr) > 1000000000ul;
}

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

bool setupInfluxDB() {
  // Add default tags
  WriteOptions wo;
  //wo.batchSize(2).bufferSize(4).maxRetryAttempts(0);
  wo.addDefaultTag(F("clientId"), WiFi.getHostname());
  wo.addDefaultTag(F("Device"), F(TOSTRING(DEVICE_HW)));
  wo.addDefaultTag(F("Version"), F(VERSION));
  wo.addDefaultTag(F("WiFi"), WiFi.SSID());
  client.setWriteOptions(wo);

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
    return true;
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
    return false;
  }
}

bool writeSensors() {
  Point sen("environment");
  if (!client.getServerUrl().length()) {
    return false;
  }

  //Serial.println(String(F("InfluxDB server connected: ")) + String(client.isConnected()));

  // Report all measurements
  sen.addField(F("TempGreenhouse"), sensors.getAvg( sensors.sum.owGreenTemp));
  sen.addField(F("TempInternal"), sensors.getAvg( sensors.sum.owIntTemp));
  sen.addField(F("TempOutdoor"), sensors.getAvg( sensors.sum.owOutTemp));
  sen.addField(F("Temperature"), sensors.getAvg( sensors.sum.shtTemp));
  sen.addField(F("Humidity"), sensors.getAvg( sensors.sum.shtHum));
  sen.addField(F("SPL06Temp"), sensors.getAvg( sensors.sum.SPL06Temp));
  sen.addField(F("Pressure"), sensors.getAvg( sensors.sum.SPL06Press));
  sen.addField(F("VCC"), sensors.getAvg( sensors.sum.VCC));
  sen.addField(F("Lux"), sensors.getAvg( sensors.sum.lux));
  sen.addField(F("ActorVoltage"), sensors.getAvg( sensors.sum.actorVoltage));
  sen.addField(F("ActorCurrent"), sensors.getAvg( sensors.sum.actorCurrent));
  if ( sensors.sum.actorMaxCurrent > 0)
    sen.addField(F("ActorMaxCurrent"), sensors.sum.actorMaxCurrent);
  sen.addField(F("WindSpeed"), sensors.getAvg( sensors.sum.windSpeed));
  sen.addField(F("WindDir"), sensors.getWindDirAvg());
  sen.addField(F("Rain"), sensors.sum.rain.sum); //exception - send sum, not average
  sen.addField(F("IsRaining"), sensors.isRaining());
  sen.addField(F("StrongWind"), sensors.strongWind());
  sen.addField(F("Window"), actor.windowCurrentState);

  // Print what are we exactly writing
  Serial.print(F("Writing: "));
  Serial.println(client.pointToLineProtocol(sen));

  // Write point
  bool res = client.writePoint(sen);
  if (!res) {
    Serial.print(F("InfluxDB write failed: "));
    Serial.println(client.getLastErrorMessage());
  }

  if ( !client.flushBuffer()) {
    Serial.print(F("InfluxDB flush failed: "));
    Serial.println(client.getLastErrorMessage());
    res = false;
  }

  if (res || (client.getLastStatusCode() > 0)) { //successful write or some http error code received (skip only IP connection issues)?
    writeSuccess++;
  }

  return res;
}

void checkWrites() {
  if (writeSuccess == 0)
    ESP.restart();
  writeSuccess = 0;
}

bool writeStatus( const char* s) {
  bool res = true;
  if(!client.getServerUrl().length()) {
    return false;
  }

  //Serial.println(String(F("InfluxDB server connected: ")) + String(client.isConnected()));

  Point status("device_status");
  status.addTag(F("Event"), s);
  status.addField(F("free_heap"), ESP.getFreeHeap());
  status.addField(F("max_alloc_heap"), ESP.getMaxAllocHeap());
  //status.addField(F("heap_fragmentation"), ESP.getHeapFragmentation());
  status.addField(F("uptime"), millis()/1000.0);
  status.addField(F("relay"), actor.getActorRelay());
  status.addField(F("auto"), actor.windowAuto);
  status.addField(F("window"), actor.getActorRelay() > 0 ? actor.windowTargetState : actor.windowCurrentState);

  Serial.print(F("Writing device status: "));
  Serial.println(client.pointToLineProtocol(status));

  if (!client.writePoint(status)) {
    Serial.print(F("InfluxDB write failed: "));
    Serial.println(client.getLastErrorMessage());
    res = false;
  }

  if (!client.flushBuffer()) {
    Serial.print(F("InfluxDB flush failed: "));
    Serial.println(client.getLastErrorMessage());
    res = false;
  }

  return res;
}