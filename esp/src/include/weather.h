#ifndef SRC_INCLUDE_WEATHER_H_
#define SRC_INCLUDE_WEATHER_H_

void weather_refreshData();

void weather_setWoeid(uint32_t woeid);
uint32_t weather_getWoeid();

uint32_t weather_getFetchTime();

#endif /* SRC_INCLUDE_WEATHER_H_ */
