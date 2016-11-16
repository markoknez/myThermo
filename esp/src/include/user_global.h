#include "u8g.h"
#include "user_interface.h"
#include "espconn.h"
#include "mqtt.h"
#include "auto_temp.h"

extern u8g_t u8g;


typedef enum {
    MANUAL,
    AUTOMATIC
} temperatureControlMode;


MQTT_Client mqttClient;
extern uint32_t secondsFromRestart;
extern uint32_t unixSeconds;
extern bool npt_invalid;

auto_state_t *currentState;
auto_state_t *states;
uint16_t states_len;
extern temperatureControlMode temperatureMode;
extern int32_t ntpTimeOffset;

extern int16_t manual_temp;

extern uint32_t led_on;

extern bool show_display;
extern bool wifi_connected;
extern bool heater_enabled;

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

Weather weather;

void ntp_init(void);
void ntp_connect(void);
uint8_t ntp_get_day_of_week();
uint32_t ntp_get_current_time(void);
void ntp_set_time_offset(int32_t offset);
void ntp_get_time_string(char *buffer);


void drawing_init(void);
void draw_screen(void);

typedef bool (* webserver_handler)(struct espconn *conn, UrlFrame *url);

void webserver_init(void);
void webserver_register_handler(webserver_handler handler);
void data_send_json(struct espconn *conn, char *json);
void data_send_text(struct espconn *conn, char *text);
void data_send_fail(struct espconn *conn, uint16_t code, char *statusMessage);
void data_send_ok(struct espconn *conn);
