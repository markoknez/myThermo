#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__

#define MQTT_SSL_ENABLE

/*DEFAULT CONFIGURATIONS*/

#ifndef MQTT_DEVICEID
#define MQTT_DEVICEID "termo-1"
#endif

#define MQTT_HOST     "ec2.mrostudios.com" //or "mqtt.yourdomain.com"
#define MQTT_PORT     1883
#define MQTT_BUF_SIZE   2048
#define MQTT_KEEPALIVE    120  /*second*/

#define MQTT_CLIENT_ID    MQTT_DEVICEID
#define MQTT_USER     "termo-1"
#define MQTT_PASS     "123456"
#define MQTT_CLEAN_SESSION 1
#define MQTT_KEEPALIVE 120

#define STA_SSID "a+mNET"
#define STA_PASS "rtm29a+m"

#define OLED_SDA_MUX    PERIPHS_IO_MUX_GPIO4_U
#define OLED_SDA_FUNC   FUNC_GPIO4
#define OLED_SDA_GPIO   4
#define OLED_SCL_MUX    PERIPHS_IO_MUX_MTMS_U
#define OLED_SCL_FUNC   FUNC_GPIO14
#define OLED_SCL_GPIO   14

#define RELAY_MUX   PERIPHS_IO_MUX_GPIO2_U
#define RELAY_FUNC  FUNC_GPIO2
#define RELAY_GPIO  2

#define TEMP_MUX    PERIPHS_IO_MUX_GPIO5_U
#define TEMP_FUNC   FUNC_GPIO5
#define TEMP_GPIO   5

#define MQTT_RECONNECT_TIMEOUT  5 /*second*/

#define DEFAULT_SECURITY  0
#define QUEUE_BUFFER_SIZE       2048

#define PROTOCOL_NAMEv311 /*MQTT version 3.1 compatible with Mosquitto v0.15*/
//PROTOCOL_NAMEv311     /*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/

#endif // __MQTT_CONFIG_H__
