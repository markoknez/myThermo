/*
 * json_parse_weather.h
 *
 *  Created on: 16. lis 2015.
 *      Author: Marko
 */

#ifndef IOT_DEMO_INCLUDE_JSON_PARSE_WEATHER_H_
#define IOT_DEMO_INCLUDE_JSON_PARSE_WEATHER_H_

typedef struct {
    uint8_t code;
    int8_t temp;
    char *text;
} weather_response_t;

extern weather_response_t weather_response;

void json_parse_weather(char *json);

#endif /* IOT_DEMO_INCLUDE_JSON_PARSE_WEATHER_H_ */
