#include "icons.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_global.h"
#include "json_parse_weather.h"
#include "upgrade.h"

u8g_t u8g;
os_timer_t timer_drawing;
bool show_display = true;
bool display_sleeping = false;

ICACHE_FLASH_ATTR
static void draw_weather_icon() {
    switch (weather_response.code) {
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
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_40);
            break;
        case 5:
        case 6:
        case 7:
        case 35:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_16);
            break;
        case 8:
        case 10:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_19);
            break;
        case 9:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_13);
            break;
        case 11:
        case 12:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_10);
            break;
        case 13:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_25);
            break;
        case 14:
        case 15:
        case 16:
        case 41:
        case 42:
        case 43:
        case 46:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_22);
            break;
        case 17:
        case 18:
        case 19:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_19);
            break;
        case 20:
        case 21:
        case 22:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_28);
            break;
        case 23:
        case 24:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_34);
            break;
        case 25:
        case 26:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_0);
            break;
        case 27:
        case 29:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_3);
            break;
        case 28:
        case 30:
        case 44:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_2);
            break;
        case 31:
        case 33:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_46);
            break;
        case 32:
        case 34:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_41);
            break;
        case 36:
            icon_draw_center(&u8g, 20, 16 + (48 / 2), icon_62);
            break;
        default:
            break;
    }
}

ICACHE_FLASH_ATTR
static void draw_normal(void) {
    uint8_t buf[50];
    u8g_SetColorIndex(&u8g, 1);

    //header
    if (heater_enabled) {
        icon_draw_center_right(&u8g, 128, 8, icon_fire);
    }
    if (wifi_connected) {
        icon_draw_center_right(&u8g, 128 - icon_fire_width, 8, icon_wifi);
    }

    //weather icon
    if (weather_response.code != 255) {
        draw_weather_icon();
    }

    uint8_t iconWidth = 40;

    //text
    //time text
//    u8g_SetFont(&u8g, u8g_font_6x10);
//    u8g_SetFontPosTop(&u8g);
//    ntp_get_time_string(buf);
//    os_sprintf(buf, "%s|%d", buf, ntp_get_day_of_week());
    uint8_t timeStringHeight = 0;    //u8g_GetFontBBXHeight(&u8g);
//    uint8_t timeStringWidth = u8g_GetStrWidth(&u8g, buf);
//    u8g_DrawStr(&u8g, iconWidth + (128 - iconWidth) / 2 - timeStringWidth / 2, 16, buf);

    //current measured temperature
    u8g_SetFont(&u8g, u8g_font_helvB18);
    u8g_SetFontPosTop(&u8g);
    os_sprintf(buf, "%d", temperature / 100);
    uint8_t currentTempWidth = u8g_GetStrWidth(&u8g, buf);
    uint8_t cWidth = u8g_GetStrWidth(&u8g, "°C");
    uint8_t strHeight = u8g_GetFontAscent(&u8g);

    u8g_SetFont(&u8g, u8g_font_helvR08);
    u8g_SetFontPosBaseline(&u8g);
    os_sprintf(buf, "%02d", abs(temperature % 100));
    uint8_t webTempWidth = u8g_GetStrWidth(&u8g, buf);
    u8g_DrawStr(&u8g, iconWidth + (128 - iconWidth) / 2 - (currentTempWidth + cWidth + webTempWidth) / 2 + currentTempWidth, 16 + strHeight, buf);

    u8g_SetFont(&u8g, u8g_font_helvB18);
    u8g_SetFontPosTop(&u8g);
    os_sprintf(buf, "%d", temperature / 100);
    u8g_DrawStr(&u8g, iconWidth + (128 - iconWidth) / 2 - (currentTempWidth + cWidth + webTempWidth) / 2, 16, buf);
    u8g_DrawStr(&u8g, iconWidth + (128 - iconWidth) / 2 - (currentTempWidth + cWidth + webTempWidth) / 2 + currentTempWidth + webTempWidth, 16, "°C");

    //web text
    if (weather_response.code != 255) {
        u8g_SetFont(&u8g, u8g_font_6x10);
        u8g_SetFontPosBottom(&u8g);
        os_sprintf(buf, "%s", weather_response.text);
        currentTempWidth = u8g_GetStrWidth(&u8g, buf);
        strHeight = u8g_GetFontBBXHeight(&u8g);
        u8g_DrawStr(&u8g, iconWidth + (128 - iconWidth) / 2 - currentTempWidth / 2, 64, buf);
    }

    //web temperature
    u8g_SetFont(&u8g, u8g_font_6x10);
    u8g_SetFontPosBottom(&u8g);
    os_sprintf(buf, "%d°C", weather_response.temp);
    uint8_t outsideTempWidth = u8g_GetStrWidth(&u8g, buf);
    u8g_DrawStr(&u8g, iconWidth + (128 - iconWidth) / 2 - outsideTempWidth / 2, 64 - strHeight, buf);

//    uint16_t hours = secondsFromRestart / 3600;
//    uint16_t minutes = (secondsFromRestart - hours * 3600) / 60;
//    uint16_t seconds = (secondsFromRestart - hours * 3600 - minutes * 60);
//    os_sprintf(buf, "%02d:%02d:%02d", hours, minutes, seconds);
//
//    u8g_SetFontPosTop(&u8g);
//    u8g_DrawStr(&u8g, 48, 16, "time up:");
//    u8g_DrawStr(&u8g, 48, 16 + fontHeight, buf);
//    u8g_DrawStr(&u8g, 48, 16 + fontHeight * 2, "current time:");
    u8g_SetFont(&u8g, u8g_font_6x10);
    u8g_SetFontPosCenter(&u8g);

    if(temperatureMode == MANUAL){
        os_sprintf(buf, "Manual: %d.%02d°C", manual_temp / 100, abs(manual_temp % 100));
    } else if (temperatureMode == AUTOMATIC){
        if(currentState == NULL){
            os_sprintf(buf, "Auto");
        }
        else{
            os_sprintf(buf, "A:%d.%02d°C, %d:%02d", currentState->temp / 100, abs(currentState->temp % 100), currentState->time / 60, currentState->time % 60);
        }
    }

    u8g_DrawStr(&u8g, 0, 8, buf);

}

ICACHE_FLASH_ATTR
static void draw_upgrade(void) {
    u8g_SetFont(&u8g, u8g_font_6x10);
    u8g_SetFontPosCenter(&u8g);
    u8g_DrawStr(&u8g, 0, 8, "upgrading fw...");
    icon_draw_center(&u8g, 128 / 2, 16 + (48 / 2), icon_update);
}

bool upgrade = false;

ICACHE_FLASH_ATTR
void draw_screen(void) {
    if (!show_display) {
        if (!display_sleeping) {
            u8g_SleepOn(&u8g);
            display_sleeping = true;
        }
        return;
    }

    if (show_display && display_sleeping) {
        display_sleeping = false;
        u8g_SleepOff(&u8g);
    }
    u8g_FirstPage(&u8g);
    do {
        if (system_upgrade_flag_check() == UPGRADE_FLAG_START || upgrade) {
            upgrade = true;
            draw_upgrade();
        } else {
            draw_normal();
        }
    } while (u8g_NextPage(&u8g));
}

ICACHE_FLASH_ATTR
void drawing_init(void) {
    u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x64_i2c, u8g_com_null_fn);

    os_timer_disarm(&timer_drawing);
    os_timer_setfn(&timer_drawing, draw_screen, NULL);
    os_timer_arm(&timer_drawing, 1000, 1);
}
