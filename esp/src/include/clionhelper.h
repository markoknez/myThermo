//
// Created by marko on 22.10.2016..
//

#ifndef TERMOSTAT_CLIONHELPER_H
#define TERMOSTAT_CLIONHELPER_H

#ifdef CLION
#define ets_sprintf(...) nothing(0, __VA_ARGS__)
#define os_printf_plus(...) nothing(0, __VA_ARGS__)
#define ets_memcpy(...) nothing(0, __VA_ARGS__)
#define ets_memmove(...) nothing(0, __VA_ARGS__)
#define ets_memset(...) nothing(0, __VA_ARGS__)
#define strcat(...) nothing(0, __VA_ARGS__)
#define strchr(...) nothing(0, __VA_ARGS__)
#define ets_strcmp(...) nothing(0, __VA_ARGS__)
#define ets_strcpy(...) nothing(0, __VA_ARGS__)
#define ets_strlen(...) nothing(0, __VA_ARGS__)
#define ets_strncmp(...) nothing(0, __VA_ARGS__)
#define ets_strncpy(...) nothing(0, __VA_ARGS__)
#define ets_strstr(...) nothing(0, __VA_ARGS__)
#define ets_timer_arm_new(...) nothing(0, __VA_ARGS__)
#define ets_timer_disarm(...) nothing(0, __VA_ARGS__)
#define ets_timer_setfn(...) nothing(0, __VA_ARGS__)

#define ets_bzero(...) nothing(0, __VA_ARGS__)
#define us ets_delay_us(...) nothing(0, __VA_ARGS__)
#define ets_install_putc1(...) nothing(0, __VA_ARGS__)

#define vPortFree(...) nothing(0, __VA_ARGS__)
#define pvPortMalloc(...) nothing(0, __VA_ARGS__)
#define pvPortCalloc(...) nothing(0, __VA_ARGS__)
#define pvPortRealloc(...) nothing(0, __VA_ARGS__)
#define pvPortZalloc(...) nothing(0, __VA_ARGS__)
#endif

int nothing(int x, ...);

#endif //TERMOSTAT_CLIONHELPER_H
