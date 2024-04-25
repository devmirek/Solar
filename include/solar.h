#ifndef SOLAR_H
#define SOLAR_H

#define VERSION "0.01"

//DEFAULT CONSTANTS
#define REFRESH_INFLUXDB_MIN 1 //Set send to InfluxDB rate

#define OFF_TEMPERATURE 24
#define FAN_TEMPERATURE 28
#define MAX_TEMPERATURE 32
#define SENSOR_READ_INTERVAL_SEC 2

//Default 1-wire sensors addresses
#define OW_ROOM   { 0x28, 0x38, 0xf3, 0x49, 0xf6, 0xd4, 0x3c, 0xeb}
#define OW_COOLER { 0x28, 0x7e, 0x33, 0x49, 0xf6, 0x04, 0x3c, 0x56}

//Time settings
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"
#define NTP_SERVERS "tik.cesnet.cz,time.google.com,time.nis.gov"

//InfluxDB Settings
#define INFLUXDB_URL "<url>"
#define INFLUXDB_TOKEN "<token>"
#define INFLUXDB_ORG "<org>"
#define INFLUXDB_BUCKET "<bucket>"

#include "custom_dev.h" //Custom development configuration - remove or comment it out

#define STRINGIZE(x) #x
#define STR_VALUE(x) STRINGIZE(x)

#endif //SOLAR_H