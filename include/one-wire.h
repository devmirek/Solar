#ifndef ONE_WIRE_H
#define ONE_WIRE_H

class OneWireSensors {
public:
  enum class tTempSensor { tRoom, tSolarCooler};  //Temp sensors
  void setup();
  float getTemp( tTempSensor channel);
  String getOWStrWeb();
  String log();
  void dumpSensors();
private:
  String strAddress(const uint8_t* deviceAddress, bool shortStr);
};

extern OneWireSensors owSensors;

#endif //ONE_WIRE_H