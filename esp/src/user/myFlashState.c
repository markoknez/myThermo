#include "osapi.h"
#include "mem.h"

#include "user_global.h"
#include "myFlashState.h"
#include "my_flash.h"


uint32_t SECTOR_STATUS = 0xFB;
uint32_t SECTOR_STATES = 0xFC;

ICACHE_FLASH_ATTR
void save_status() {
    char data[12];
    uint32_t weather_woeid = 851128;//weather_getWoeid();
    my_flash_writeValue16(data, states_len);
    my_flash_writeValue16(data + 2, ntpTimeOffset);
    data[4] = temperatureMode;
    my_flash_writeValue16(data + 5, manual_temp);
    my_flash_writeValue32(data + 7, weather_woeid);
    data[11] = 0;

    my_flash_write(SECTOR_STATUS, data, 12);
}

ICACHE_FLASH_ATTR
void read_status() {
    char *data = NULL;
    uint32_t len = 0;
    my_flash_read(SECTOR_STATUS, &data, &len);
    if (data == NULL)
        return;

    uint32_t weather_woeid = 0;
    states_len = my_flash_readValue16(data);
    ntpTimeOffset = my_flash_readValue16(data + 2);
    temperatureMode = data[4];
    manual_temp = my_flash_readValue16(data + 5);
    weather_woeid = my_flash_readValue32(data + 7);
    //weather_setWoeid(weather_woeid);
    os_free(data);
}

ICACHE_FLASH_ATTR
void save_states() {
    //TODO: hack because states_len is saved in status field
    save_status();
    my_flash_write(SECTOR_STATES, (char *) states, states_len * 4);
}

ICACHE_FLASH_ATTR
void read_states() {
    auto_state_t *newStates = NULL;
    uint32_t len;
    //TODO: check how this knows the len!?
    my_flash_read(SECTOR_STATES, (char **) &newStates, &len);

    if (newStates == NULL)
        return;

    if (states != NULL)
        os_free(states);
    states = newStates;
}