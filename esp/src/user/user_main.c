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
#include <drawing.h>
#include "osapi.h"

#include "fota.h"
#include "myFlashState.h"
#include "mqttMain.h"

DrawingState drawingState;

uint32_t secondsFromRestart = 0;
os_timer_t secondTimer;

TemperatureControlMode temperatureMode = MANUAL;

auto_state_t *currentState = NULL;
MQTT_Client mqttClient;

LOCAL ICACHE_FLASH_ATTR
void set_heater(uint16_t targetTemp, uint16_t currentTemp) {
    if (currentTemp < targetTemp) {
        drawingSetHeaterEnabled(&drawingState, true);
    } else {
        drawingSetHeaterEnabled(&drawingState, false);
    }


    GPIO_OUTPUT_SET(GPIO_ID_PIN(RELAY_GPIO), drawingState.heaterEnabled ? 0 : 1);
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
        set_heater(drawingState.manualTemp, temperature_currentTemperature);
    } else if (temperatureMode == AUTOMATIC) {
        currentState = findCurrentState();
        //no state found, switch back to manual
        if (currentState == NULL) {
            temperatureMode = MANUAL;
            drawingSetTempeartureMode(&drawingState, temperatureMode);
            mqttPublishMode(&mqttClient, 'm');
            return;
        }
        set_heater(currentState->temp, temperature_currentTemperature);
        drawingSetAutomaticTempCurrentState(&drawingState, currentState);
    }
}

int16_t lastSentTemperature = 0;

LOCAL ICACHE_FLASH_ATTR
void second_counter(void) {
    secondsFromRestart++;
    unixSeconds++;

    if (mqttClient.pCon && lastSentTemperature != temperature_currentTemperature) {
        mqttPublishCurrentTemp(&mqttClient, temperature_currentTemperature);
        lastSentTemperature = temperature_currentTemperature;
        drawingSetCurrentTemp(&drawingState, temperature_currentTemperature);
    }

    if (mqttClient.pCon && secondsFromRestart % 10 == 0) {
        mqttPublishUptime(&mqttClient, secondsFromRestart);
    }

    temperatureEngine();
}

LOCAL ICACHE_FLASH_ATTR
void wifi_status_handler(System_Event_t *evt) {
    switch (evt->event) {
        case EVENT_STAMODE_GOT_IP:
            os_printf("connecting to mqtt\n");
            MQTT_Connect(&mqttClient);
            drawingSetWifiConnected(&drawingState, true);
            ntp_init();
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
            drawingSetWifiConnected(&drawingState, false);
            break;
        default:
            break;
    }
}

//        struct rst_info *info = system_get_rst_info();
//        os_printf("restar reason:\n");
//        os_printf("reason: %d\n", info->reason);
//        os_printf("exccause: %d\n", info->exccause);
//        os_printf("epc1: %d\n", info->epc1);
//        os_printf("epc2: %d\n", info->epc2);
//        os_printf("epc3: %d\n", info->epc3);
//        os_printf("excvaddr: %d\n", info->excvaddr);
//        os_printf("depc: %d\n", info->depc);

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
    PIN_FUNC_SELECT(RELAY_MUX, RELAY_FUNC);

    os_printf("SDK version:%s\n\n", system_get_sdk_version());

    temperature_init();
    temperature_startReading();


    //start seconds timer
    os_timer_disarm(&secondTimer);
    os_timer_setfn(&secondTimer, second_counter, NULL);
    os_timer_arm(&secondTimer, 1000, 1);


    struct station_config config;
    config.bssid_set = 0;  //do net check MAC address of AP
    os_strncpy(config.ssid, STA_SSID, 32);
    os_strncpy(config.password, STA_PASS, 64);

    read_status();
    read_states();

    mqttStart();

    wifi_set_opmode_current(0x01);  //set station mode
    wifi_set_event_handler_cb(wifi_status_handler);
    wifi_station_set_config_current(&config);
    wifi_station_connect();

    //start duckdns update
    //duckdns_update();

    //start thingspeak update
    //os_timer_disarm(&timer_thingSpeak);
    //os_timer_setfn(&timer_thingSpeak, thingSpeak_update, NULL);
    //os_timer_arm(&timer_thingSpeak, THINGSPEAK_UPDATE_PERIOD_MS, 1);

    //WIFI_Connect("a+mNET", "rtm29a+m", wifi_cb);

    //start webserver
    //webserver_register_handler(user_webserver_handler);
    //webserver_init();
    drawingInit(&drawingState);
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
