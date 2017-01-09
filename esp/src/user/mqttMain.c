#include <user_global.h>
#include <fota.h>
#include <drawing.h>
#include <mqtt.h>
#include "osapi.h"
#include "mem.h"
#include "auto_temp.h"
#include "myFlashState.h"
#include "mqttMain.h"


#ifndef BUILD_TIME
#error "BUILD_TIME not defined"
#endif

auto_state_t *states = NULL;
uint16_t states_len = 0;

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


static ICACHE_FLASH_ATTR
void user_rf_pre_init(void) {
}


//static void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status) {
//    if (status == STATION_GOT_IP) {
//        MQTT_Connect(&mqttClient);
//    } else {
//        MQTT_Disconnect(&mqttClient);
//    }
//}

void mqttPublishFWVersion(MQTT_Client *mqttClient) ;

static void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args) {
    MQTT_Client *client = (MQTT_Client *) args;
    INFO("MQTT: Connected to %s\r\n", client->host);
    INFO("MQTT: DeviceId [%s], Username [%s], Password [%s]\r\n", client->connect_info.client_id, client->connect_info.username, client->connect_info.password);
    INFO("MQTT: Topics: %s\r\n", "mrostudios/devices/"MQTT_DEVICEID"/#");
    MQTT_Subscribe(client, "mrostudios/devices/"MQTT_DEVICEID"/autoTemp/command", 1);
    MQTT_Subscribe(client, "mrostudios/devices/"MQTT_DEVICEID"/manualTemp/command", 1);
    MQTT_Subscribe(client, "mrostudios/devices/"MQTT_DEVICEID"/mode/command", 1);
    MQTT_Subscribe(client, "mrostudios/devices/"MQTT_DEVICEID"/upgrade/command", 1);
    MQTT_Subscribe(client, "mrostudios/devices/"MQTT_DEVICEID"/tempCalibration/command", 1);
    MQTT_Subscribe(client, "mrostudios/weather/851128/status", 1);

    mqttPublishFWVersion(client);
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
static void handlerWeather(MQTT_Client *mqttClient, const char *topic, const char *data) {
    char *strIdx = data;
    uint32_t colonIdx = 0;
    char temp[64];
    Weather weather;

    if (os_strlen(data) > 64) {
        INFO("Weather data too long\n");
        return;
    }

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
    drawingSetWeather(&drawingState, &weather);
}

ICACHE_FLASH_ATTR
static void handleManualTemp(MQTT_Client *mqttClient, const char *topic, const char *data) {
    INFO("Received manual temp: %s\n", data);
    int16_t manualTemp = atoi(data);
    temperatureEngine();
    save_status();
    MQTT_Publish(mqttClient, "mrostudios/devices/"MQTT_DEVICEID"/manualTemp/status", data, os_strlen(data), 1, 1);
    drawingSetManualTemp(&drawingState, manualTemp);
}

ICACHE_FLASH_ATTR
static void handleAutoTemp(MQTT_Client *mqttClient, const char *topic, const char *data) {
    auto_state_t *newStates = autoTempDecode(data, strlen(data), &states_len);
    if (newStates != NULL) {
        if (states) os_free(states);
        states = newStates;
        save_states();
        MQTT_Publish(mqttClient, "mrostudios/devices/"MQTT_DEVICEID"/autoTemp/status", data, os_strlen(data), 1, 1);
    }
}

ICACHE_FLASH_ATTR
static void handleMode(MQTT_Client *mqttClient, const char *topic, const char *data) {
    TemperatureControlMode temperatureMode;
    if (os_strstr(data, "m"))
        temperatureMode = MANUAL;
    else if (os_strstr(data, "a"))
        temperatureMode = AUTOMATIC;
    save_status();

    MQTT_Publish(mqttClient, "mrostudios/devices/"MQTT_DEVICEID"/mode/status", data, os_strlen(data), 1, 1);
    drawingSetTempeartureMode(&drawingState, temperatureMode);
}

ICACHE_FLASH_ATTR
static void handleUpgradeMessage(MQTT_Client *mqttClient, char *buf, char *dataBuf) {
    INFO("Received upgrade command, starting upgrade %s\n", dataBuf);
    char ip[] = {192, 168, 1, 2};
    handleUpgrade(2, ip, 8080, dataBuf);
    drawingSetUpgrade(&drawingState, true);

}

static void handleTempCalibration(MQTT_Client *client, char *topic, char *message) {
    INFO("Temperature calibration - %s\n", message);

    tempCalibration = atoi(message);
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
        handlerWeather(client, topicBuf, dataBuf);
    } else if (os_strstr(topicBuf, "mrostudios/devices/"MQTT_DEVICEID"/manualTemp/command")) {
        handleManualTemp(client, topicBuf, dataBuf);
    } else if (os_strstr(topicBuf, "mrostudios/devices/"MQTT_DEVICEID"/autoTemp/command")) {
        handleAutoTemp(client, topicBuf, dataBuf);
    } else if (os_strstr(topicBuf, "mrostudios/devices/"MQTT_DEVICEID"/mode/command")) {
        handleMode(client, topicBuf, dataBuf);
    } else if (os_strstr(topicBuf, "mrostudios/devices/"MQTT_DEVICEID"/upgrade/command")) {
        handleUpgradeMessage(client, topicBuf, dataBuf);
    } else if (os_strstr(topicBuf, "mrostudios/devices/"MQTT_DEVICEID"/tempCalibration/command")) {
        handleTempCalibration(client, topicBuf, dataBuf);
    } else {
        INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
    }

    os_free(topicBuf);
    os_free(dataBuf);

}

static void ICACHE_FLASH_ATTR print_info() {
    INFO("\r\n\r\n[INFO] BOOTUP...\r\n");
    INFO("[INFO] SDK: %s\r\n", system_get_sdk_version());
    INFO("[INFO] Chip ID: %08X\r\n", system_get_chip_id());
    INFO("[INFO] Memory info:\r\n");
    system_print_meminfo();

    INFO("[INFO] -------------------------------------------\n");
    INFO("[INFO] Build time: %s\n", BUILD_TIME);
    INFO("[INFO] -------------------------------------------\n");
}


void ICACHE_FLASH_ATTR mqttStart(MQTT_Client *mqttClient) {
    print_info();
//    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    MQTT_InitConnection(mqttClient, MQTT_HOST, MQTT_PORT, DEFAULT_SECURITY);
    //MQTT_InitConnection(mqttClient, "192.168.11.122", 1880, 0);

    MQTT_InitClient(mqttClient, MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS, MQTT_KEEPALIVE, MQTT_CLEAN_SESSION);
    //MQTT_InitClient(mqttClient, "client_id", "user", "pass", 120, 1);
    MQTT_InitLWT(mqttClient, "mrostudios/devices/"MQTT_DEVICEID"/lastWill", "offline", 1, 0);
    MQTT_OnConnected(mqttClient, mqttConnectedCb);
    MQTT_OnDisconnected(mqttClient, mqttDisconnectedCb);
    MQTT_OnPublished(mqttClient, mqttPublishedCb);
    MQTT_OnData(mqttClient, mqttDataCb);
}

void mqttPublishCurrentTemp(MQTT_Client *mqttClient, int16_t temperature) {
    char buf[16];
    os_sprintf(buf, "%d.%02d", temperature / 100, abs(temperature % 100));
    MQTT_Publish(mqttClient, "mrostudios/devices/"MQTT_DEVICEID"/currentTemp/status", buf, os_strlen(buf), 1, 1);
}

void mqttPublishMode(MQTT_Client *mqttClient, char mode) {
    char *str = "\0\0";
    str[0] = mode;
    MQTT_Publish(mqttClient, "mrostudios/devices/"MQTT_DEVICEID"/mode/status", str, 1, 1, 1);
}

void mqttPublishUptime(MQTT_Client *mqttClient, uint32_t uptime) {
    char buf[16];
    os_sprintf(buf, "%d", uptime);
    MQTT_Publish(mqttClient, "mrostudios/devices/"MQTT_DEVICEID"/uptime/status", buf, os_strlen(buf), 1, 1);
}

bool lastHeaterState = false;
void mqttPublishHeater(MQTT_Client *mqttClient, bool isEnabled) {
    if(isEnabled && lastHeaterState == false)
        MQTT_Publish(mqttClient, "mrostudios/devices/"MQTT_DEVICEID"/heater/status", "true", 4, 1, 1);

    if(!isEnabled && lastHeaterState == true)
        MQTT_Publish(mqttClient, "mrostudios/devices/"MQTT_DEVICEID"/heater/status", "false", 5, 1, 1);

    lastHeaterState = isEnabled;
}

void mqttPublishFWVersion(MQTT_Client *mqttClient) {
    MQTT_Publish(mqttClient, "mrostudios/devices/"MQTT_DEVICEID"/fwVersion/status", BUILD_TIME, os_strlen(BUILD_TIME), 1, 1);
}