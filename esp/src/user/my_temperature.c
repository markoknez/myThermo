//
// Created by marko on 15.8.2016..
//

#include "ets_sys.h"
#include "osapi.h"
#include "my_temperature.h"
#include "driver/ds1820.h"

os_timer_t temperature_readTimer;
uint8_t temperature_deviceId[8];
bool temperature_deviceFound = false;

ICACHE_FLASH_ATTR
void temperature_init(void) {
    ds_init();
    ds_reset();
    ds_reset_search();
    if (ds_search(temperature_deviceId)) {
        os_printf("DS18B20 device found... - ");
        unsigned char i;
        for (i = 0; i < 8; i++)
            os_printf("0x%x ", temperature_deviceId[i]);
        os_printf("\n");
        temperature_deviceFound = true;
    }
    else os_printf("temperature device not found!!!!!!!!!!!!!\n");
}

ICACHE_FLASH_ATTR
void temperature_readTemperature(void) {
    ds_request_temp(temperature_deviceId);
}

ICACHE_FLASH_ATTR
void temperature_startReading(void) {
    os_timer_disarm(&temperature_readTimer);
    os_timer_setfn(&temperature_readTimer, temperature_readTemperature, NULL);
    os_timer_arm(&temperature_readTimer, 5000, 1);
}

ICACHE_FLASH_ATTR
void temperature_stopReading(void) {
    os_timer_disarm(&temperature_readTimer);
}