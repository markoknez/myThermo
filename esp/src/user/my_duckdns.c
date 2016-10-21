#include <c_types.h>
#include <http_client.h>
#include <osapi.h>
#include <os_type.h>
#include "my_duckdns.h"

os_timer_t duck_timer;

ICACHE_FLASH_ATTR
void duckdns_update(void){
    http_get("http://www.duckdns.org/update/mrostudios/e00dada5-2820-493c-9277-e20c8f3ff48d", "", duckdns_response);
}

ICACHE_FLASH_ATTR
void duckdns_response(char * response_body, int http_status, char * full_response){
    if(http_status == 200){
        if(os_strcmp(response_body, "OK") == 0){
            os_printf("DuckDNS updated\n");
            os_timer_disarm(&duck_timer);
            os_timer_arm(&duck_timer, DUCKDNS_TIMEOUT_MS, 1);
            return;
        }
    }
    os_printf("DuckDNS update failed\n");
    os_timer_disarm(&duck_timer);
    //update fail, wait 5 seconds
    os_timer_arm(&duck_timer, 5000, 1);
}
