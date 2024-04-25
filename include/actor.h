#ifndef ACTOR_H
#define ACTOR_H

class WindowActor {
public:
  const int8_t windowStop = -1;
  const int8_t windowClose = 0;
  const int8_t windowOpen = 100;
  void setup();
  void loop();
  bool setWindow( int8_t state);
  uint8_t getActorRelay();
  String log( bool web);

  //TODO: move variables to private
  bool windowAuto = true;
  uint8_t windowCurrentState = 50;  //middle position to always start window positioning
  uint8_t windowTargetState = 50; //middle position to always start window positioning
  long windowMoveTime = 0;
private:
  void _setActorRelay( uint8_t dir);

  uint16_t windowFullTransitionTimeMs = DEFAULT_WINDOW_TRANSITION_TIME_MS;
};

extern WindowActor actor;


#endif //ACTOR_H