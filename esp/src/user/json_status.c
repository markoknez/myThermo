#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "user_json.h"
#include "json/json.h"
#include "time.h"
#include "json_parse_weather.h"
#include "weather.h"

#include "user_global.h"

ICACHE_FLASH_ATTR
char* json_create() {
    char *buffer = (char *)os_zalloc(2048);
    char temp[256];

    os_sprintf(buffer, "{\n");

    ntp_get_time_string(temp);
    os_sprintf(buffer, "%s\t \"time\":\"%s\"\n", buffer, temp);

    time_t time = ntp_get_current_time();
    struct tm *t_time = localtime(&time);
    os_sprintf(buffer, "%s\t,\"date\":\"%02d/%02d/%02d\"\n", buffer, t_time->tm_mday, t_time->tm_mon + 1, t_time->tm_year + 1900);


    os_sprintf(buffer, "%s\t,\"temp\":%d.%02d\n", buffer, temperature / 100, abs(temperature % 100));

    uint8_t hour = secondsFromRestart / 3600;
    uint8_t minute = (secondsFromRestart - hour * 3600) / 60;
    uint8_t second = secondsFromRestart % 60;
    os_sprintf(buffer, "%s\t,\"up_time\":\"%02d:%02d:%02d\"\n", buffer, hour, minute, second);

    os_sprintf(buffer, "%s\t,\"outside_temp\":%d\n", buffer, weather_response.temp);

    os_sprintf(buffer, "%s\t,\"outside_text\":\"%s\"\n", buffer, weather_response.text);

    if(heater_enabled)
       os_sprintf(buffer, "%s\t,\"heater_enabled\":true\n", buffer);
   else
       os_sprintf(buffer, "%s\t,\"heater_enabled\":false\n", buffer);

    if(temperatureMode == MANUAL)
        os_sprintf(buffer, "%s\t, \"temperatureMode\":\"manual\"\n", buffer);
    else
        os_sprintf(buffer, "%s\t, \"temperatureMode\":\"automatic\"\n", buffer);

    os_sprintf(buffer, "%s\t,\"manual_temp\":%d.%02d\n", buffer, manual_temp / 100, abs(manual_temp % 100));

    os_sprintf(buffer, "%s\t,\"time_offset\":%d\n", buffer, ntpTimeOffset);

    os_sprintf(buffer, "%s\t, \"weatherWoeid\":%d\n", buffer, weather_getWoeid());

    os_sprintf(buffer, "%s}\n", buffer);

    return buffer;
}

ICACHE_FLASH_ATTR
char* json_status_get() {
    return json_create();
}
