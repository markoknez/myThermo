#include "osapi.h"
#include "ets_sys.h"
#include "weather.h"
#include "http_client.h"
#include "json_parse_weather.h"
#include "user_global.h"

uint32_t weather_fetchTime = 0;
uint32_t weather_woeid = 851128;
bool weather_fetching = false;

ICACHE_FLASH_ATTR
static void get_weather_response_handler(char * response_body, int http_status, char * full_response) {
    if(http_status == 200){
        json_parse_weather(response_body);
        weather_fetchTime = secondsFromRestart;
        os_printf("weather refreshed...\n");
    }
    else {
        os_printf("weather refreshed failed: %d\n%s\n", http_status, full_response);
    }
    weather_fetching = false;
}

ICACHE_FLASH_ATTR
void weather_setWoeid(uint32_t woeid){
    weather_woeid = woeid;
}
ICACHE_FLASH_ATTR
uint32_t weather_getWoeid(){
    return weather_woeid;
}

ICACHE_FLASH_ATTR
uint32_t weather_getFetchTime(){
    return weather_fetchTime;
}

ICACHE_FLASH_ATTR
void weather_refreshData(void) {
    if(weather_fetching)
        return;

    weather_fetching = true;
    uint8_t url[256];
    os_sprintf(url, "http://query.yahooapis.com/v1/public/yql?q=select%%20item.condition%%20from%%20weather.forecast%%20where%%20woeid=%d%%20and%%20u=%%27c%%27&format=json", weather_woeid);

    http_get(url,
            "",
            get_weather_response_handler);
}


