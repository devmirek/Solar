#include <OneWire.h>
#include <DallasTemperature.h>
#include "one-wire.h"
#include "solar.h"

OneWireSensors owSensors;
OneWire oneWire(PIN_ONE_WIRE);
DallasTemperature ow_sensors(&oneWire);

DeviceAddress internalThermometer = OW_INTERNAL;
DeviceAddress outdoorThermometer = OW_OUTDOOR;
DeviceAddress greenhouseThermometer = OW_GREENHOUSE;

void OneWireSensors::setup() {
  ow_sensors.begin(); // Start up the library

  uint8_t oneWireDevices = ow_sensors.getDeviceCount();
  if (oneWireDevices > 0) {
    Serial.print("Found thermometers: ");
    Serial.println(oneWireDevices);
  } else
    Serial.println("Error: Missing 1-wire thermometer!");
  ow_sensors.setResolution( 12); //12-bit resolution
}

String OneWireSensors::strAddress( const uint8_t* deviceAddress, bool shortStr) {
  String s;
  for (uint8_t i = 0; i < 8; i++) {
    if (!shortStr)
      s += "0x";
    if (deviceAddress[i] < 16)     // zero pad the address if necessary
      s += "0";
    s += String(deviceAddress[i], HEX);
    if (!shortStr)
      if ( i != 7)
        s += ", ";
  }
  return s;
}

float OneWireSensors::getTemp( tTempSensor channel) {
 //Read temperature from the DS1820 sensor(s)
  ow_sensors.requestTemperatures();
  DeviceAddress* adr = &greenhouseThermometer;
  switch (channel) {
  case tTempSensor::tInternal:
    adr = &internalThermometer;
    break;
  case tTempSensor::tOutdoor:
    adr = &outdoorThermometer;
    break;
  case tTempSensor::tGreenHouse:  //already assigned
    break;
  }
  float t = ow_sensors.getTempC(*adr);
  if (t != -127)
    return t;
  return NAN;
  /*for ( uint8_t i1 = 0; i1 < ow_sensors.getDeviceCount(); i1++) {
    t = ow_sensors.getTempCByIndex( i1);
    //ow_sensors.getAddress(insideThermometer, i1);
    //printOWAddress( insideThermometer); Serial.println();
    if (t == -127)
      return NAN;
    return t;
  }
  return NAN;*/
}

String OneWireSensors::getOWStrWeb() {
  String s;

  ow_sensors.requestTemperatures();
  uint8_t oneWireDevices = ow_sensors.getDeviceCount();

  for ( uint8_t i1 = 0; i1 < oneWireDevices; i1++) {
    float t = ow_sensors.getTempCByIndex( i1);
    if (t == -127)
      t = NAN;
    DeviceAddress adr;
    ow_sensors.getAddress(adr, i1);
    if (memcmp( adr, greenhouseThermometer, sizeof(adr)) == 0)
    s += String("<br/>OWTemp") + i1 + ": " + t + " °C " + strAddress(adr, true);
  }
  return s;
}


String OneWireSensors::log() {
  float t = getTemp( tTempSensor::tInternal);
  Serial.println(String("Temp(Internal): ") + t + "°C");
  t = getTemp( tTempSensor::tOutdoor);
  Serial.println(String("Temp(Outdoor): ") + t + "°C");
  t = getTemp( tTempSensor::tGreenHouse);
  Serial.println(String("Temp(Greenhouse): ") + t + "°C");
  return "";
}