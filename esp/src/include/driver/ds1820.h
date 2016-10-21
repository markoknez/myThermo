#ifndef ESP8266_NONOS_SDK_MY_DS1820_H
#define ESP8266_NONOS_SDK_MY_DS1820_H

#include <stdint.h>

void ds_init(void);

uint8_t ds_reset(void);

void ds_reset_search();

uint8_t ds_search( uint8_t *newAddr );

void ds_request_temp(uint8_t *address);

#endif