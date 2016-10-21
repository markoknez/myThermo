#include <c_types.h>
#include <osapi.h>
#include <http_client.h>
#include <stdlib.h>
#include "my_temperature.h"
#include "my_thingspeak.h"

ICACHE_FLASH_ATTR
void thingSpeak_response(char *response_body, int http_status, char *full_response){
    if(http_status != 200)
        os_printf("thingspeak failed, status code: %d\n", http_status);
}

ICACHE_FLASH_ATTR
void thingSpeak_update(void){
    os_printf("updating thingspeak\n");
    char request[256];
    os_sprintf(request, "http://api.thingspeak.com/update.json?api_key=B4Z9ZXQU6WYVQ93A&field1=%d.%02d", temperature_currentTemperature / 100, abs(
            temperature_currentTemperature % 100));
    http_get(request, "", thingSpeak_response);
}