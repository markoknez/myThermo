#ifndef ESP8266_NONOS_SDK_MY_DUCKDNS_H
#define ESP8266_NONOS_SDK_MY_DUCKDNS_H

#define DUCKDNS_TIMEOUT_MS     5 * 60 * 1000     //5 minutes duckdns refresh time

void duckdns_response(char * response_body, int http_status, char * full_response);

void duckdns_update(void);

#endif
