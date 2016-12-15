///******************************************************************************
// * Copyright 2013-2014 Espressif Systems (Wuxi)
// *
// * FileName: user_main.c
// *
// * Description: entry file of user application
// *
// * Modification history:
// *     2014/1/1, v1.0 create this file.
//*******************************************************************************/
#include <my_temperature.h>
#include <mqtt.h>
#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#include "user_global.h"
#include "my_flash.h"
#include "fota.h"
#include "auto_temp.h"
#include "myFlashState.h"

#define THINGSPEAK_UPDATE_PERIOD_MS  30000

uint32_t secondsFromRestart = 0;
bool wifi_connected = false;
bool heater_enabled = false;
os_timer_t secondTimer;

temperatureControlMode temperatureMode = MANUAL;
int16_t manual_temp = 1800;

auto_state_t *currentState = NULL;
MQTT_Client mqttClient;


os_timer_t timer_thingSpeak;

void handle_auto_temp(const char *base64_str, int base64_len);

char *get_auto_temp(size_t *output_len);


LOCAL ICACHE_FLASH_ATTR
void set_heater(uint16_t targetTemp, uint16_t currentTemp) {
    if (currentTemp < targetTemp) {
        heater_enabled = true;
        return;
    }

    if (currentTemp > targetTemp) {
        heater_enabled = false;
        return;
    }


    GPIO_OUTPUT_SET(GPIO_ID_PIN(2), heater_enabled ? 0 : 1);
}

LOCAL ICACHE_FLASH_ATTR
auto_state_t *findCurrentState(void) {
    if (states_len == 0 || states == NULL)
        return NULL;

    uint32_t unixTime = ntp_get_current_time();
    uint8_t weekday = ntp_get_day_of_week();
    uint8_t hour = (unixTime % 86400L) / 3600;
    uint8_t min = (unixTime % 3600) / 60;
    uint16_t currentTime = weekday * 24 * 60 + hour * 60 + min;

    uint8_t i;
    for (i = 0; i < states_len; i++) {
        if (states[i].time > currentTime) {
            if (i == 0)
                return &states[states_len - 1];

            return &states[i - 1];
        }
    }
    return &states[states_len - 1];
}

ICACHE_FLASH_ATTR
void temperatureEngine(void) {
    if (temperatureMode == MANUAL) {
        set_heater(manual_temp, temperature_currentTemperature);
    } else if (temperatureMode == AUTOMATIC) {
        currentState = findCurrentState();
        //no state found, switch back to manual
        if (currentState == NULL) {
            temperatureMode = MANUAL;
            MQTT_Publish(&mqttClient, "mrostudios/devices/termo-1/mode/status", "m", 1, 0, 1);
            return;
        }
        set_heater(currentState->temp, temperature_currentTemperature);
    }
}

int16_t lastSentTempearture = 0;

LOCAL ICACHE_FLASH_ATTR
void second_counter(void) {
    secondsFromRestart++;
    unixSeconds++;

    char buf[10];
    if (mqttClient.pCon && lastSentTempearture != temperature_currentTemperature) {
        os_sprintf(buf, "%d.%02d", temperature_currentTemperature / 100, abs(temperature_currentTemperature % 100));
        MQTT_Publish(&mqttClient, "mrostudios/devices/termo-1/currentTemp/status", buf, os_strlen(buf), 0, 1);
        lastSentTempearture = temperature_currentTemperature;
    }

    if(mqttClient.pCon && secondsFromRestart % 10 == 0) {
        os_sprintf(buf, "%d", secondsFromRestart);
        MQTT_Publish(&mqttClient, "mrostudios/devices/termo-1/uptime/status", buf, os_strlen(buf), 0, 1);
    }

//    if(wifi_connected && (weather_getFetchTime() == 0 || secondsFromRestart - weather_getFetchTime() > WEATHER_TIMEOUT_S)){
//        weather_refreshData();
//    }

    temperatureEngine();
}

LOCAL ICACHE_FLASH_ATTR
void wifi_status_handler(System_Event_t *evt) {
    switch (evt->event) {
        case EVENT_STAMODE_GOT_IP:
            os_printf("connecting to mqtt\n");
            MQTT_Connect(&mqttClient);
            wifi_connected = true;
//            ntp_init();
            os_printf("ip:"
                              IPSTR
                              ",mask:"
                              IPSTR
                              ",gw:"
                              IPSTR
                              "\n", IP2STR(&evt->event_info.got_ip.ip), IP2STR(&evt->event_info.got_ip.mask),
                      IP2STR(&evt->event_info.got_ip.gw));
            break;
        case EVENT_STAMODE_DISCONNECTED:
            MQTT_Disconnect(&mqttClient);
            wifi_connected = false;
            break;
        default:
            break;
    }
}

os_timer_t test;
ICACHE_FLASH_ATTR
void test_update() {
    char ip[] = {192, 168, 1, 2};
    handleUpgrade(2, ip, 8080, "/");
}

//ICACHE_FLASH_ATTR
//static bool user_webserver_handler(struct espconn *conn, UrlFrame *frame) {
//    os_printf("free heap: %d\n", system_get_free_heap_size());
//
//    if (os_strcmp(frame->url, "/ntp_init") == 0) {
//        ntp_connect();
//        data_send_ok(conn);
//        return true;
//    } else if(os_strcmp(frame->url, "/set_contrast") == 0){
//        os_printf("setting contrast: %d\n", atoi(frame->body));
//        u8g_SetContrast(&u8g, atoi(frame->body));
//    } else if (os_strcmp(frame->url, "/command") == 0) {
//        json_parse_command(frame->body);
//        data_send_ok(conn);
//        save_status();
//        return true;
//    } else if (os_strcmp(frame->url, "/get_weather") == 0) {
//        weather_refreshData();
//        data_send_ok(conn);
//        return true;
//    } else if(os_strcmp(frame->url, "/start_upgrade") == 0){
//        os_timer_disarm(&test);
//        os_timer_setfn(&test, test_update, NULL);
//        os_timer_arm(&test, 1000 ,0);
//        data_send_ok(conn);
//        return true;
//    } else if(os_strcmp(frame->url, "/update_duckdns") == 0){
//        duckdns_update();
//        data_send_ok(conn);
//        return true;
//    } else if(os_strcmp(frame->url, "/show_alloc") == 0){
//        system_show_malloc();
//        data_send_ok(conn);
//        return true;
//    } else if(os_strcmp(frame->url, "/get_restart_reason") == 0){
//        struct rst_info *info = system_get_rst_info();
//        os_printf("restar reason:\n");
//        os_printf("reason: %d\n", info->reason);
//        os_printf("exccause: %d\n", info->exccause);
//        os_printf("epc1: %d\n", info->epc1);
//        os_printf("epc2: %d\n", info->epc2);
//        os_printf("epc3: %d\n", info->epc3);
//        os_printf("excvaddr: %d\n", info->excvaddr);
//        os_printf("depc: %d\n", info->depc);
//        data_send_ok(conn);
//        return true;
//    } else if(os_strcmp(frame->url, "/set_auto_temp") == 0){
//        handle_auto_temp(frame->body, frame->content_length);
//        data_send_ok(conn);
//        save_states();
//        return true;
//    } else if(os_strcmp(frame->url, "/get_auto_temp") == 0){
//        size_t base64_len;
//        char *base64 = get_auto_temp(&base64_len);
//        data_send_text(conn, base64);
//        os_free(base64);
//
//        return true;
//    } else if(os_strcmp(frame->url, "/set_mode/auto") == 0){
//        temperatureMode = AUTOMATIC;
//        data_send_ok(conn);
//        save_status();
//        return true;
//    } else if(os_strcmp(frame->url, "/set_mode/manual") == 0){
//        temperatureMode = MANUAL;
//        data_send_ok(conn);
//        save_status();
//        return true;
//    } else if(os_strcmp(frame->url, "/get_status") == 0){
//        char *status = json_status_get();
//        data_send_json(conn, status);
//        os_free(status);
//        return true;
//    }
//
//    return false;
//}





void start_mqtt();

static void ICACHE_FLASH_ATTR wifi_cb(uint8_t status) {
    if (status == STATION_GOT_IP) {
        MQTT_Connect(&mqttClient);
    } else {
        MQTT_Disconnect(&mqttClient);
    }
}

ICACHE_FLASH_ATTR
void app_init(void) {
    os_printf("Hello from clion!!!\n");

    gpio_init();
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

    os_printf("SDK version:%s\n\n", system_get_sdk_version());

    temperature_init();
    temperature_startReading();

    drawing_init();

    //start seconds timer
    os_timer_disarm(&secondTimer);
    os_timer_setfn(&secondTimer, second_counter, NULL);
    os_timer_arm(&secondTimer, 1000, 1);


    struct station_config config;
    config.bssid_set = 0;  //do net check MAC address of AP
    char ssid[32] = "a+mNET";
    char pass[64] = "rtm29a+m";
    os_strncpy(config.ssid, ssid, 32);
    os_strncpy(config.password, pass, 64);

    read_status();
    read_states();

    start_mqtt();

    wifi_set_opmode_current(0x01);  //set station mode
    wifi_set_event_handler_cb(wifi_status_handler);
    wifi_station_set_config_current(&config);
    wifi_station_connect();

    //start duckdns update
//    duckdns_update();

    //start thingspeak update
//    os_timer_disarm(&timer_thingSpeak);
//    os_timer_setfn(&timer_thingSpeak, thingSpeak_update, NULL);
//    os_timer_arm(&timer_thingSpeak, THINGSPEAK_UPDATE_PERIOD_MS, 1);

//    WIFI_Connect("a+mNET", "rtm29a+m", wifi_cb);

    //start webserver
//    webserver_register_handler(user_webserver_handler);
//    webserver_init();

}

///******************************************************************************
// * FunctionName : user_init
// * Description  : entry of user application, init user function here
// * Parameters   : none
// * Returns      : none
// *******************************************************************************/
void user_init(void) {
    system_init_done_cb(app_init);
}
