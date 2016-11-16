#ifndef TERMOSTAT_AUTO_TEMP_H
#define TERMOSTAT_AUTO_TEMP_H

#include "user_config_base.h"
#include "c_types.h"

typedef struct {
    uint16_t time;
    uint16_t temp;
} auto_state_t;

char *autoTempEncode(auto_state_t *states, size_t states_len, size_t *encodedLen);
auto_state_t *autoTempDecode(const char *base64_str, int base64_len, size_t *states_len);

#endif //TERMOSTAT_AUTO_TEMP_H
