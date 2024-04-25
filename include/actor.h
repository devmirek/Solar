#ifndef ACTOR_H
#define ACTOR_H

class SolarActor {
public:
  enum tSolarMode { sOff=0, sOn, sMax};
  void setup();
  bool setSolarVentilation( tSolarMode mode);
  tSolarMode getActorRelay() {return _mode;};
  uint8_t getActorRelayRaw();
  const char* getRelayStr( tSolarMode mode);
  String log( bool web);
  bool solarAuto = true;
private:
  void setActorRelay( tSolarMode mode);
  tSolarMode _mode = sOff;
};

extern SolarActor actor;


#endif //ACTOR_H