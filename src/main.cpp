//  Otevre vlhokst > 80% zavre vlkost <60% a zaroven teplota > 5st C
//https://docs.google.com/spreadsheets/d/14_UTkpEV5io6cRv7Sp_b7svRXbfMLgnT2r7b6Amcv5A/edit
#define REGISTER_WIFI_AP wifiMulti.addAP( "<WIFI SSID>", "<WIFI PASSWORD>");
//https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
//https://learn.sparkfun.com/tutorials/arduino-weather-shield-hookup-guide-v12
//https://gitlab.com/Tynet.eu/esp8266-thingsboard-weatherstation/-/blob/master/ESP8266_wetterstation_MQTT.ino

//Board: DOIT ESP32 DEVKIT V1

//TODO:
// Better current monitor handling
//  -detect reason of 0A peaks
//  -detect overload
//  -reduce number of errors (moving average?)
// Improve web interface to responsive one (use WS)
// Build own board of Window device
// Rebooted at 8am (main loop stuck?)
// SPL06 is no reported correctly
// Lux meter does not work


#include <WiFiMulti.h>
#include <WiFiClient.h>
#include <esp_task_wdt.h>
#include "solar.h"
#include "actor.h"


WiFiMulti wifiMulti;
bool bButton = true;
unsigned long timeSinceLastMinute = 0;
unsigned long timeSinceLastSensorRead = 0;
uint8_t timeMins = 0;

void setupWebServer();
void loopWebServer();
bool updateClock( bool firstStart, const char* tzInfo, const char* ntp);
bool setupInfluxDB();
bool writeStatus( const char* s);
bool writeSensors();
void checkWrites();

/*
 * setup function
 */
void setup(void) {
  Serial.begin(115200);
  esp_task_wdt_init( 60, true); //enable 60s panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  ledcSetup(CHAN_BUZZER, 8000, 12); //initialize buzzer
  ledcAttachPin(PIN_BUZZER, CHAN_BUZZER);
  ledcWriteTone(CHAN_BUZZER, 0);
  delay(1000);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite( PIN_LED, LOW);
  pinMode(PIN_BUTTON, INPUT);
  sensors.testI2C();

  // Connect to WiFi network
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char hostSuffix[5];
  snprintf(hostSuffix, 5, "%02x%02x", mac[4], mac[5]);

  Serial.println("Version: " VERSION " " __DATE__ " " __TIME__);
  Serial.println("Host: " "window-" + String(hostSuffix));

  WiFi.hostname("window-" + String(hostSuffix));
  WiFi.mode(WIFI_STA);
  REGISTER_WIFI_AP;
  // Wait for connection
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  updateClock( true, TZ_INFO, NTP_SERVERS);

  //initialize InfluxDB
  setupInfluxDB();
  writeStatus( "boot");

  //initialize sensors
  owSensors.setup();
  sensors.setup();

  //initialize web server
  setupWebServer();

  actor.setup();
  weather.setup();
  ledcWriteTone(CHAN_BUZZER, 2000);   //beep
  delay(200);
  ledcWriteTone(CHAN_BUZZER, 0);
}

bool logPin( uint8_t pin, bool &state, const char* name) {
  if ( digitalRead(pin) != state) {
    state = !state;
    Serial.println( String(name) + (state ? " DOWN" : " UP"));
    return state;
  }
  return false;
}

void loop(void) {
  esp_task_wdt_reset();
  loopWebServer();
  actor.loop();
  digitalWrite( PIN_LED, wifiMulti.run() == WL_CONNECTED ? HIGH : LOW); //Set blue led according to WiFi status
  //check WiFi
  if (wifiMulti.run() != WL_CONNECTED)
    Serial.println("WiFi not connected!");

  if ((millis() - timeSinceLastMinute) >= 1000*60) {  //call it once a minute
    timeSinceLastMinute = millis();
    timeMins++;
    Serial.println("controlling window");

    //------------ call once a minute  ------------
    //Read temperature from the DS1820 or other sensors
    float t = sensors.getGreenhouseTemp();
    Serial.println("Temp(Greenhouse) control: " + String(t) + "°C");

    //manage window status
    if ( actor.windowAuto) {  //only in case auto is enabled
      if (sensors.isRaining() || sensors.strongWind()) { //close window if it is raining or wind is too strong
        actor.setWindow( actor.windowClose);
        Serial.println("close window due to rain or wind");
      } else if ( t >= MAX_TEMPERATURE)
        actor.setWindow( actor.windowOpen);
      else if ( t <= MIN_TEMPERATURE)
        actor.setWindow( actor.windowClose);
    }

    //------- call it as often as possible (last statement) ------
    if (timeMins == 60) {   //once an hour
      esp_task_wdt_reset();
      Serial.println("writing status");
      timeMins = 0; //reset min counter
      if (wifiMulti.run() != WL_CONNECTED)  //disconnected wifi? reboot
        ESP.restart();
      writeStatus( "hourly");
      checkWrites();
    }

    esp_task_wdt_reset();
    sensors.logSensors( false);
    owSensors.log();

    writeSensors(); //write all sensors
    sensors.clearSumSensors();
  } //once per minute

  //------- call it as often as possible ------
  if ( logPin( PIN_BUTTON, bButton, "Button")) {
    Serial.println("Controlling window");
    if (actor.windowMoveTime == 0) { //not moving - jump to opposite state
      actor.setWindow( actor.windowCurrentState == actor.windowOpen ? actor.windowClose : actor.windowOpen);
    } else {  //moving? - stop immediately
      actor.setWindow( actor.windowStop);
    }
  }

  if ((millis() - timeSinceLastSensorRead) >= 1000*SENSOR_READ_INTERVAL_SEC) {  //read sensors values
    timeSinceLastSensorRead = millis();
    esp_task_wdt_reset();
    sensors.readSensors();
  }
  delay(1);
}
