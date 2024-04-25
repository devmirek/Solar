#ifndef SOLAR_H
#define SOLAR_H

#define VERSION "0.01"

//DEFAULT CONSTANTS
#define REFRESH_WINDOW_MIN 1 //Set window rate
#define REFRESH_INFLUXDB_MIN 1 //Set send to InfluxDB rate

#define MIN_CURRENT 0.5
#define MAX_CURRENT 1.8

#define MIN_TEMPERATURE 24
#define MAX_TEMPERATURE 28
#define RAIN_RECOVERY_TIME_SEC (5*60)  //how long after detected rain should keep window closed
#define WIND_RECOVERY_TIME_SEC (5*60)  //how long after detected rain should keep window closed
#define WIND_MAX_KM_H 50

#define DEFAULT_WINDOW_TRANSITION_TIME_MS (45 * 1000)


#define MIN_WIND_SPEED_INTERVAL_SEC 30
#define SENSOR_READ_INTERVAL_SEC 2

//Default 1-wire sensors addresses
#define OW_INTERNAL   { 0x28, 0xDD, 0xCA, 0xC7, 0x01, 0x00, 0x00, 0x1B}
#define OW_OUTDOOR    { 0x28, 0xAC, 0x3E, 0x76, 0xE0, 0x01, 0x3C, 0xA6}
#define OW_GREENHOUSE { 0x28, 0x65, 0x2B, 0x76, 0xE0, 0x01, 0x3C, 0x01}

//Time settings
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"
#define NTP_SERVERS "tik.cesnet.cz,time.google.com,time.nis.gov"


//InfluxDB Settings
#define INFLUXDB_URL "<url>"
#define INFLUXDB_TOKEN "<token>"
#define INFLUXDB_ORG "<org>"
#define INFLUXDB_BUCKET "<bucket>"

#include "custom_dev.h" //Custom development configuration - remove or comment it out
#endif //WINDOW_H