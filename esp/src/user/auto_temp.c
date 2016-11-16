#include "auto_temp.h"
#include "osapi.h"
#include "mem.h"

char *autoTempEncode(auto_state_t *states, size_t states_len, size_t *encodedLen) {
    int data_cnt = 0;
    char data[states_len * 4 + 2];
    int i, placeholder_for_day_state_count, day = 0;
    int state_day_count = 0;

    data[data_cnt++] = states_len >> 8;
    data[data_cnt++] = states_len & 0xff;
    for (i = 0; i < states_len; i++) {
        data[data_cnt++] = states[i].time >> 8;
        data[data_cnt++] = states[i].time & 0xff;
        data[data_cnt++] = states[i].temp >> 8;
        data[data_cnt++] = states[i].temp & 0xff;
    }

    return base64_encode2(data, sizeof(data), encodedLen);
}

auto_state_t *autoTempDecode(const char *base64_str, int base64_len, size_t *states_len){
    size_t data_len;
    char *data;
    INFO("decoding base64\n");
    data = base64_decode2(base64_str, base64_len, &data_len);

    int data_pos = 0;

    size_t new_states_len = (data[data_pos++] << 8) | data[data_pos++];
    INFO("found states %d\n", new_states_len);
    if(new_states_len > 7 * 48){
        INFO("too many states\n");
        os_free(data);
        return NULL;
    }

    auto_state_t *new_states = (auto_state_t *) os_zalloc(sizeof(auto_state_t) * new_states_len);


    int state_index;
    for(state_index = 0; state_index < new_states_len; state_index++){
        if(data_pos + 4 > data_len || state_index >= new_states_len){
            INFO("bad auto_temp request 3\n");
            os_free(data);
            os_free(new_states);
            return NULL;
        }

        new_states[state_index].time = (data[data_pos] << 8) | data[data_pos + 1];
        data_pos += 2;
        new_states[state_index].temp = (data[data_pos] << 8) | data[data_pos + 1];
        data_pos += 2;
    }

    *states_len = new_states_len;

    return new_states;
}