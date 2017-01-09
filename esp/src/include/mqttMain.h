#ifndef TERMOSTAT_MQTTMAIN_H
#define TERMOSTAT_MQTTMAIN_H

extern DrawingState drawingState;
extern int16_t tempCalibration;

void mqttStart(MQTT_Client *mqttClient);

void mqttPublishCurrentTemp(MQTT_Client *mqttClient, int16_t temperature);
void mqttPublishMode(MQTT_Client *mqttClient, char mode);
void mqttPublishUptime(MQTT_Client *mqttClient, uint32_t uptime);
void mqttPublishHeater(MQTT_Client *mqttClient, bool isEnabled);

#endif //TERMOSTAT_MQTTMAIN_H
