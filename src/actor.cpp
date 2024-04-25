#include <Arduino.h>
#include "solar.h"
#include "actor.h"

WindowActor actor;
bool writeStatus( const char* s);

enum tWindowDirection { wIdle=0, wClosing, wOpening};

void WindowActor::_setActorRelay( uint8_t dir) {
  bool bUp = HIGH;
  if (dir == wOpening)
    bUp = LOW;
  digitalWrite( PIN_RELAY1, bUp);
  digitalWrite( PIN_RELAY2, bUp);
  writeStatus("relay");
}

uint8_t WindowActor::getActorRelay() {
  return digitalRead( PIN_RELAY1) | (digitalRead( PIN_RELAY2) < 1);
}

void WindowActor::setup() {
  pinMode(PIN_RELAY1, OUTPUT);
  pinMode(PIN_RELAY2, OUTPUT);
  _setActorRelay( wIdle);
}

bool WindowActor::setWindow( int8_t state) {  //state - 0(closed)..100(opened)
  Serial.println("setWindow: " + String(state) + " current: " + String(windowCurrentState));

  if ( state == windowStop) { //stop relay immediately
    windowMoveTime = millis();  //stop the time
    _setActorRelay( wIdle); //just to make sure
    return false;
  }

  if (state > windowOpen) //limit
    state = windowOpen;

  if ( state == windowTargetState) //if state equals, skip
    return true;

  if ( windowMoveTime != 0) {//ignore new value if already in transition
    Serial.println("Already in transition, stopping!");
    _setActorRelay( wIdle);
    return false;
  }

  Serial.println("setWindow from " + String(windowCurrentState) + " to " + String(state));
  windowTargetState = state;  //save new state
  windowMoveTime = millis() + (abs(windowCurrentState - state) * windowFullTransitionTimeMs / 100);

  Serial.println("setWindow time " + String(millis()) + " to target " + String(windowMoveTime));

  if ( state > windowCurrentState) {  //opening window
    Serial.println("Window opening from " + String(windowCurrentState) + " to " + String(state));
    _setActorRelay( wOpening);
  } else {  //closing window
    Serial.println("Window closing from " + String(windowCurrentState) + " to " + String(state));
    _setActorRelay( wClosing);
  }
  return true;
}

void WindowActor::loop() { //process loop to close window when needed
  if ( windowMoveTime == 0)  //no activity
    return;

  if ( windowMoveTime > millis() + windowFullTransitionTimeMs + (windowFullTransitionTimeMs / 10)) { //trim time if any issue
    Serial.println("Error, trimming " + String(millis()) + " move " + String(windowMoveTime));
    windowMoveTime = millis() + windowFullTransitionTimeMs;
  }

  //TODO: check current, if 0 switch off
  if ( millis() >= windowMoveTime) { //limit reached, release relays
    //TODO: if current is still above zero, add 30% of the whole time
    Serial.println("Window done");
    windowMoveTime = 0;
    windowCurrentState = windowTargetState;
    _setActorRelay( wIdle);
  }
}

String WindowActor::log( bool web) { //log all sensors
  String s = "Window current: " + String(windowCurrentState) + " Target: " + String(windowTargetState) +
             " Time: " + String(windowMoveTime == 0 ? 0 : windowMoveTime - millis());

  return s;
}