#ifndef ESP8266_NONOS_SDK_MY_THINGSPEAK_H
#define ESP8266_NONOS_SDK_MY_THINGSPEAK_H

void thingSpeak_response(char *response_body, int http_status, char *full_response);

void thingSpeak_update(void);

#endif
