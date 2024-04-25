#include <OneWire.h>
#include <DallasTemperature.h>
#include "one-wire.h"
#include "solar.h"

OneWireSensors owSensors;
OneWire oneWire(PIN_ONE_WIRE);
DallasTemperature ow_sensors(&oneWire);

DeviceAddress roomThermometer = OW_ROOM;
DeviceAddress solarCoolerThermometer = OW_COOLER;

void OneWireSensors::setup() {
  ow_sensors.begin(); // Start up the library

  uint8_t oneWireDevices = ow_sensors.getDeviceCount();
  if (oneWireDevices > 0) {
    Serial.print("Found thermometers: ");
    Serial.println(oneWireDevices);
  } else
    Serial.println("Error: Missing 1-wire thermometer!");
  ow_sensors.setResolution( 12); //12-bit resolution
  dumpSensors();
}

String OneWireSensors::strAddress( const uint8_t* deviceAddress, bool shortStr) {
  String s;
  s.reserve(20);
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
  DeviceAddress* adr = &solarCoolerThermometer;
  switch (channel) {
  case tTempSensor::tRoom:
    adr = &roomThermometer;
    break;
  case tTempSensor::tSolarCooler:
    adr = &solarCoolerThermometer;
    break;
  }
  float t = ow_sensors.getTempC(*adr);
  if (t != -127)
    return t;
  return NAN;
}

void OneWireSensors::dumpSensors() {
  for ( uint8_t i1 = 0; i1 < ow_sensors.getDeviceCount(); i1++) {
    float t = ow_sensors.getTempCByIndex( i1);
    if (t == -127)
      t = NAN;
    DeviceAddress adr;
    ow_sensors.getAddress(adr, i1);
    Serial.println( String(i1) + " " + strAddress(adr, false) + " " + String(t));
  }
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
    if (memcmp( adr, solarCoolerThermometer, sizeof(adr)) == 0)
      s += String("Cooler: <b>") + t + "°C</b> " + strAddress(adr, true) + "<br>";
    else
      s += String("Room: ") + t + "°C " + strAddress(adr, true) + "<br>";
  }
  return s;
}

String OneWireSensors::log() {
  float t = getTemp( tTempSensor::tRoom);
  Serial.println(String("Temp(Room): ") + t + "°C");
  t = getTemp( tTempSensor::tSolarCooler);
  Serial.println(String("Temp(Solar Cooler): ") + t + "°C");
  return "";
}