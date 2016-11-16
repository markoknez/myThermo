#include <user_global.h>
#include "osapi.h"
#include "mem.h"
#include "auto_temp.h"

ICACHE_FLASH_ATTR
uint32 user_rf_cal_sector_set(void) {
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


LOCAL ICACHE_FLASH_ATTR
void user_rf_pre_init(void) {
}


static void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status) {
    if (status == STATION_GOT_IP) {
        MQTT_Connect(&mqttClient);
    } else {
        MQTT_Disconnect(&mqttClient);
    }
}

static void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args) {
    MQTT_Client *client = (MQTT_Client *) args;
    INFO("MQTT: Connected\r\n");
    MQTT_Subscribe(client, "mrostudios/weather/851128/status", 0);
    MQTT_Subscribe(client, "mrostudios/termo-1/manualTemp", 0);
    MQTT_Subscribe(client, "mrostudios/termo-1/autoTemp", 0);
    MQTT_Subscribe(client, "mrostudios/termo-1/mode", 0);
}

static void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args) {
    MQTT_Client *client = (MQTT_Client *) args;
    INFO("MQTT: Disconnected\r\n");
}

static void ICACHE_FLASH_ATTR mqttPublishedCb(uint32_t *args) {
    MQTT_Client *client = (MQTT_Client *) args;
    INFO("MQTT: Published\r\n");
}

ICACHE_FLASH_ATTR
void handlerWeather(const char *topic, const char *data) {
    char *strIdx = data;
    uint32_t colonIdx = 0;
    char temp[64];

    char *index = os_strstr(data, ";");
    if (index == NULL) return;
    colonIdx = index - strIdx;
    os_strncpy(temp, strIdx, colonIdx);
    temp[colonIdx] = 0;
    strIdx += colonIdx + 1;
    weather.code = atoi(temp);

    index = os_strstr(strIdx, ";");
    if (index == NULL) return;
    colonIdx = index - strIdx;
    os_strncpy(temp, strIdx, colonIdx);
    temp[colonIdx] = 0;
    strIdx += colonIdx + 1;
    weather.temp = atoi(temp);

    index = os_strstr(strIdx, ";");
    if (index == NULL) return;
    colonIdx = index - strIdx;
    os_strncpy(weather.text, strIdx, colonIdx);
    weather.text[colonIdx] = 0;

    INFO("Received weather: %d / %d / %s\n", weather.code, weather.temp, weather.text);

}

ICACHE_FLASH_ATTR
void handleManualTemp(const char *topic, const char *data) {
    manual_temp = atoi(data);
}

ICACHE_FLASH_ATTR
void handleAutoTemp(const char* topic, const char *data) {
    auto_state_t *newStates = autoTempDecode(data, strlen(data), &states_len);
    if(newStates != NULL) {
        os_free(states);
        states = newStates;
    }
}

ICACHE_FLASH_ATTR
void handleMode(const char* topic, const char *data) {
    if(os_strstr(data, "m"))
        temperatureMode = MANUAL;
    else if(os_strstr(data, "a"))
        temperatureMode = AUTOMATIC;
}

static void ICACHE_FLASH_ATTR
mqttDataCb(uint32_t *args, const char *topic, uint32_t topic_len, const char *data, uint32_t data_len) {
    char *topicBuf = (char *) os_zalloc(topic_len + 1);
    char *dataBuf = (char *) os_zalloc(data_len + 1);

    MQTT_Client *client = (MQTT_Client *) args;
    os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;
    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    if (os_strstr(topicBuf, "weather/851128/status")) {
        handlerWeather(topicBuf, dataBuf);
    } else if (os_strstr(topicBuf, "mrostudios/termo-1/manualTemp")) {
        handleManualTemp(topicBuf, dataBuf);
    } else if (os_strstr(topicBuf, "mrostudios/termo-1/autoTemp")) {
        handleAutoTemp(topicBuf, dataBuf);
    } else if (os_strstr(topicBuf, "mrostudios/termo-1/mode")) {
        handleMode(topicBuf, dataBuf);
    } else {
        INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
    }

    os_free(topicBuf);
    os_free(dataBuf);

}

void ICACHE_FLASH_ATTR print_info() {
    INFO("\r\n\r\n[INFO] BOOTUP...\r\n");
    INFO("[INFO] SDK: %s\r\n", system_get_sdk_version());
    INFO("[INFO] Chip ID: %08X\r\n", system_get_chip_id());
    INFO("[INFO] Memory info:\r\n");
    system_print_meminfo();

    INFO("[INFO] -------------------------------------------\n");
    INFO("[INFO] Build time: %s\n", BUILD_TIME);
    INFO("[INFO] -------------------------------------------\n");
}


void ICACHE_FLASH_ATTR start_mqtt(void) {
    print_info();
//    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    MQTT_InitConnection(&mqttClient, MQTT_HOST, MQTT_PORT, DEFAULT_SECURITY);
    //MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);

    MQTT_InitClient(&mqttClient, MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS, MQTT_KEEPALIVE, MQTT_CLEAN_SESSION);
    //MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);
    MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
    MQTT_OnConnected(&mqttClient, mqttConnectedCb);
    MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
    MQTT_OnPublished(&mqttClient, mqttPublishedCb);
    MQTT_OnData(&mqttClient, mqttDataCb);
}