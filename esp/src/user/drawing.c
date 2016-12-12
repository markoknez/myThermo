#include <drawing.h>

#include "icons.h"
#include "osapi.h"

ICACHE_FLASH_ATTR static void drawingSetInvalid(DrawingState *state) {
    state->invalid = true;
}

ICACHE_FLASH_ATTR void drawingSetShow(DrawingState *state, bool isShown) {
    if (isShown)
        u8g_SleepOn(&(state->u8g));
    else
        u8g_SleepOff(&(state->u8g));
    drawingSetInvalid(state);
}

ICACHE_FLASH_ATTR void drawingSetWeather(DrawingState *state, Weather *weather) {
    state->weather = *weather;
    drawingSetInvalid(state);
}

ICACHE_FLASH_ATTR void drawingSetCurrentTemp(DrawingState *state, int16_t newTemp) {
    state->currentTemp = newTemp;
    drawingSetInvalid(state);
}

ICACHE_FLASH_ATTR void drawingSetTempeartureMode(DrawingState *state, TemperatureControlMode newMode) {
    state->temperatureMode = newMode;
    drawingSetInvalid(state);
}

ICACHE_FLASH_ATTR void drawingSetManualTemp(DrawingState *state, int16_t newTemp) {
    state->manualTemp = newTemp;
    drawingSetInvalid(state);
}

ICACHE_FLASH_ATTR void drawingSetAutomaticTempCurrentState(DrawingState *state, auto_state_t *newState) {
    state->automaticTempCurrentState = newState;
    drawingSetInvalid(state);
}

ICACHE_FLASH_ATTR void drawingSetHeaterEnabled(DrawingState *state, bool isEnabled) {
    state->heaterEnabled = isEnabled;
    drawingSetInvalid(state);
}

ICACHE_FLASH_ATTR void drawingSetWifiConnected(DrawingState *state, bool isConnected) {
    state->wifiConnected = isConnected;
    drawingSetInvalid(state);
}

ICACHE_FLASH_ATTR void drawingSetUpgrade(DrawingState *state, bool isUpgrade) {
    state->upgrade = isUpgrade;
    drawingSetInvalid(state);
}

ICACHE_FLASH_ATTR
static void draw_weather_icon(DrawingState *state) {
    switch (state->weather.code) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 37:
        case 38:
        case 39:
        case 40:
        case 45:
        case 47:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_40);
            break;
        case 5:
        case 6:
        case 7:
        case 35:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_16);
            break;
        case 8:
        case 10:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_19);
            break;
        case 9:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_13);
            break;
        case 11:
        case 12:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_10);
            break;
        case 13:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_25);
            break;
        case 14:
        case 15:
        case 16:
        case 41:
        case 42:
        case 43:
        case 46:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_22);
            break;
        case 17:
        case 18:
        case 19:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_19);
            break;
        case 20:
        case 21:
        case 22:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_28);
            break;
        case 23:
        case 24:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_34);
            break;
        case 25:
        case 26:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_0);
            break;
        case 27:
        case 29:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_3);
            break;
        case 28:
        case 30:
        case 44:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_2);
            break;
        case 31:
        case 33:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_46);
            break;
        case 32:
        case 34:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_41);
            break;
        case 36:
            icon_draw_center(&(state->u8g), 20, 16 + (48 / 2), icon_62);
            break;
        default:
            break;
    }
}

ICACHE_FLASH_ATTR
static void drawingRefreshIcons(DrawingState *state) {
    if (state->heaterEnabled) {
        icon_draw_center_right(&(state->u8g), 128, 8, icon_fire);
    }
    if (state->wifiConnected) {
        icon_draw_center_right(&(state->u8g), 128 - icon_fire_width, 8, icon_wifi);
    }

    if (state->weather.code != 255)
        draw_weather_icon(state);
}

ICACHE_FLASH_ATTR
static void draw_normal(DrawingState *state) {
    uint8_t buf[50];
    u8g_SetColorIndex(&(state->u8g), 1);

    drawingRefreshIcons(state);

    uint8_t iconWidth = 40;
    //current measured temperature
    u8g_SetFont(&(state->u8g), u8g_font_helvB18);
    u8g_SetFontPosTop(&(state->u8g));
    os_sprintf(buf, "%d", state->currentTemp / 100);
    uint8_t currentTempWidth = u8g_GetStrWidth(&(state->u8g), buf);
    uint8_t cWidth = u8g_GetStrWidth(&(state->u8g), "C");
    uint8_t strHeight = u8g_GetFontAscent(&(state->u8g));

    u8g_SetFont(&(state->u8g), u8g_font_helvR08);
    u8g_SetFontPosBaseline(&(state->u8g));
    os_sprintf(buf, "%02d", abs(state->currentTemp % 100));
    uint8_t webTempWidth = u8g_GetStrWidth(&(state->u8g), buf);
    u8g_DrawStr(&(state->u8g),
                iconWidth + (128 - iconWidth) / 2 - (currentTempWidth + cWidth + webTempWidth) / 2 + currentTempWidth,
                16 + strHeight, buf);

    u8g_SetFont(&(state->u8g), u8g_font_helvB18);
    u8g_SetFontPosTop(&(state->u8g));
    os_sprintf(buf, "%d", state->currentTemp / 100);
    u8g_DrawStr(&(state->u8g), iconWidth + (128 - iconWidth) / 2 - (currentTempWidth + cWidth + webTempWidth) / 2, 16,
                buf);
    u8g_DrawStr(&(state->u8g),
                iconWidth + (128 - iconWidth) / 2 - (currentTempWidth + cWidth + webTempWidth) / 2 + currentTempWidth +
                webTempWidth, 16, "C");

    //web text
    if (state->weather.code != 255) {
        u8g_SetFont(&(state->u8g), u8g_font_6x10);
        u8g_SetFontPosBottom(&(state->u8g));
        os_sprintf(buf, "%s", state->weather.text);
        currentTempWidth = u8g_GetStrWidth(&(state->u8g), buf);
        strHeight = u8g_GetFontBBXHeight(&(state->u8g));
        u8g_DrawStr(&(state->u8g), iconWidth + (128 - iconWidth) / 2 - currentTempWidth / 2, 64, buf);
    }

    //web temperature
    u8g_SetFont(&(state->u8g), u8g_font_6x10);
    u8g_SetFontPosBottom(&(state->u8g));
    os_sprintf(buf, "%dC", state->weather.temp);
    uint8_t outsideTempWidth = u8g_GetStrWidth(&(state->u8g), buf);
    u8g_DrawStr(&(state->u8g), iconWidth + (128 - iconWidth) / 2 - outsideTempWidth / 2, 64 - strHeight, buf);

    u8g_SetFont(&(state->u8g), u8g_font_6x10);
    u8g_SetFontPosCenter(&(state->u8g));

    if (state->temperatureMode == MANUAL) {
        os_sprintf(buf, "Manual: %d.%02dC", state->manualTemp / 100, abs(state->manualTemp % 100));
    } else if (state->temperatureMode == AUTOMATIC) {
        if (state->automaticTempCurrentState == NULL) {
            os_sprintf(buf, "Auto");
        } else {
            os_sprintf(buf, "A:%d.%02dC, %d:%02d", state->automaticTempCurrentState->temp / 100,
                       abs(state->automaticTempCurrentState->temp % 100),
                       state->automaticTempCurrentState->time / 60, state->automaticTempCurrentState->time % 60);
        }
    }

    u8g_DrawStr(&(state->u8g), 0, 8, buf);

}

ICACHE_FLASH_ATTR
static void draw_upgrade(DrawingState *state) {
    u8g_SetFont(&(state->u8g), u8g_font_6x10);
    u8g_SetFontPosCenter(&(state->u8g));
    u8g_DrawStr(&(state->u8g), 0, 8, "upgrading fw...");
    icon_draw_center(&(state->u8g), 128 / 2, 16 + (48 / 2), icon_update);
}

ICACHE_FLASH_ATTR
static void drawingDrawScreen(void *args) {
    DrawingState *state = (DrawingState *) args;
    if (!state->invalid)
        return;

    u8g_FirstPage(&(state->u8g));
    do {
        if (state->upgrade) {
            draw_upgrade(state);
        } else {
            draw_normal(state);
        }
    } while (u8g_NextPage(&(state->u8g)));
}

ICACHE_FLASH_ATTR
void drawingInit(DrawingState *state) {
    state->invalid = false;
    state->automaticTempCurrentState = NULL;
    state->currentTemp = 1800;
    state->manualTemp = 1800;
    state->temperatureMode = MANUAL;
    state->weather.code = 255;
    state->weather.temp = 1800;
    state->upgrade = false;
    os_sprintf(state->weather.text, "");

    u8g_InitComFn(&(state->u8g), &u8g_dev_ssd1306_128x64_i2c, u8g_com_null_fn);

    os_timer_disarm(&(state->timer_drawing));
    os_timer_setfn(&(state->timer_drawing), drawingDrawScreen, state);
    os_timer_arm(&(state->timer_drawing), 1000, 1);
}
