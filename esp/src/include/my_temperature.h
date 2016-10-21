#ifndef ESP8266_NONOS_SDK_MY_TEMPERATURE_H
#define ESP8266_NONOS_SDK_MY_TEMPERATURE_H

#include "os_type.h"

extern int16_t temperature_currentTemperature;

void temperature_init(void);

void temperature_startReading(void);

void temperature_stopReading(void);

#endif
