#ifndef ONE_WIRE_H
#define ONE_WIRE_H

class OneWireSensors {
public:
  enum class tTempSensor { tInternal, tOutdoor, tGreenHouse};  //Temp sensors
  void setup();
  float getTemp( tTempSensor channel);
  String getOWStrWeb();
  String log();

private:
  String strAddress(const uint8_t* deviceAddress, bool shortStr);
};

extern OneWireSensors owSensors;

#endif //ONE_WIRE_H