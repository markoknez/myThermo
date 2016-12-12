#ifndef TERMOSTAT_USER_GLOBAL_H
#define TERMOSTAT_USER_GLOBAL_H

#include "u8g.h"
#include "user_interface.h"
#include "espconn.h"
#include "mqtt.h"
#include "auto_temp.h"

typedef enum {
    MANUAL,
    AUTOMATIC
} TemperatureControlMode;


extern uint32_t secondsFromRestart;
extern uint32_t unixSeconds;
extern bool npt_invalid;

extern auto_state_t *currentState;
extern MQTT_Client mqttClient;
extern auto_state_t *states;
extern uint16_t states_len;
extern TemperatureControlMode temperatureMode;
extern int32_t ntpTimeOffset;

extern uint32_t led_on;

typedef enum {
	GET = 0,
	POST = 1,
	OPTIONS = 2
} MethodType;

typedef struct {
	MethodType method;
	char *url;
	char *body;
	char *requestMethods;
	char *requestHeaders;
	int bodyLength;
	uint32_t content_length;
} UrlFrame;

typedef struct {
	uint8_t code;
	int16_t temp;
	char text[64];
} Weather;

void ntp_init(void);
void ntp_connect(void);
uint8_t ntp_get_day_of_week();
uint32_t ntp_get_current_time(void);
void ntp_set_time_offset(int32_t offset);
void ntp_get_time_string(char *buffer);

void temperatureEngine(void);



typedef bool (* webserver_handler)(struct espconn *conn, UrlFrame *url);

void webserver_init(void);
void webserver_register_handler(webserver_handler handler);
void data_send_json(struct espconn *conn, char *json);
void data_send_text(struct espconn *conn, char *text);
void data_send_fail(struct espconn *conn, uint16_t code, char *statusMessage);
void data_send_ok(struct espconn *conn);

#endif