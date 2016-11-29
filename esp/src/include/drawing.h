#ifndef TERMOSTAT_DRAWING_H
#define TERMOSTAT_DRAWING_H

#include "user_global.h"
#include "auto_temp.h"
#include "u8g.h"
#include "os_type.h"

typedef struct {
    Weather weather;
    int16_t currentTemp;
    TemperatureControlMode temperatureMode;
    int16_t manualTemp;
    auto_state_t *automaticTempCurrentState;
    bool heaterEnabled;
    bool wifiConnected;

    bool upgrade;

    bool invalid;

    u8g_t u8g;
    os_timer_t timer_drawing;
} DrawingState;

void drawingInit(DrawingState *state);

void drawingSetShow(DrawingState *state, bool isShown);

void drawingSetWeather(DrawingState *state, Weather *weather);

void drawingSetCurrentTemp(DrawingState *state, int16_t newTemp);

void drawingSetTempeartureMode(DrawingState *state, TemperatureControlMode newMode);

void drawingSetManualTemp(DrawingState *state, int16_t newTemp);

void drawingSetAutomaticTempCurrentState(DrawingState *state, auto_state_t *newState);

void drawingSetHeaterEnabled(DrawingState *state, bool isEnabled);

void drawingSetWifiConnected(DrawingState *state, bool isConnected);

void drawingSetUpgrade(DrawingState *state, bool isUpgrade);

#endif //TERMOSTAT_DRAWING_H
