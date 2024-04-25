#include <Arduino.h>
#include "solar.h"
#include "actor.h"

SolarActor actor;
extern bool writeStatus( const char* s);

void SolarActor::setActorRelay( tSolarMode mode) {
  if ( mode == _mode) //not changed?
    return;
  _mode = mode;
  digitalWrite( PIN_RELAY1, !(mode != sOff));
  digitalWrite( PIN_RELAY2, !(mode == sMax));
  writeStatus("relay");
}

uint8_t SolarActor::getActorRelayRaw() {
  return (!digitalRead( PIN_RELAY1)) | (!digitalRead( PIN_RELAY2) << 1);
}

const char* SolarActor::getRelayStr( tSolarMode mode) {
  switch (mode) {
    case sOff: return "off";
    case sOn: return "on";
    case sMax: return "max";
  }
  return "?";
}

void SolarActor::setup() {
  pinMode(PIN_RELAY1, OUTPUT);
  pinMode(PIN_RELAY2, OUTPUT);
  _mode = sMax; //change status to really execute relays
  setActorRelay( sOff);
}

bool SolarActor::setSolarVentilation( tSolarMode mode) {  //state - 0(closed)..100(opened)
  Serial.println( String("setSolarVentilation: ") + getRelayStr(mode));
  setActorRelay( mode);
  return true;
}

String SolarActor::log( bool web) { //log all sensors
  String s = "Solar status " + String(_mode);
  return s;
}