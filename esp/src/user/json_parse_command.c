#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "user_json.h"
#include "json/json.h"
#include "time.h"
#include "json_parse_command.h"
#include "list.h"
#include "weather.h"


#include "user_global.h"

static ICACHE_FLASH_ATTR int json_read(struct jsontree_context *js_ctx, struct jsonparse_state *parser);
//static ICACHE_FLASH_ATTR int json_read_auto_temp_item(struct jsontree_context *js_ctx, struct jsonparse_state *parser);

LOCAL struct jsontree_callback command_callback = JSONTREE_CALLBACK(NULL, json_read);
//LOCAL struct jsontree_callback auto_temp_item_callback = JSONTREE_CALLBACK(NULL, json_read_auto_temp_item);

JSONTREE_OBJECT(command_tree,
                JSONTREE_PAIR("led", &command_callback),
                JSONTREE_PAIR("time_offset", &command_callback),
                JSONTREE_PAIR("display", &command_callback),
                JSONTREE_PAIR("manual_temp", &command_callback),
                JSONTREE_PAIR("weatherWoeid", &command_callback)
                );


ICACHE_FLASH_ATTR
int json_read(struct jsontree_context *js_ctx, struct jsonparse_state *parser) {
    int type;

    while ((type = jsonparse_next(parser)) != 0) {
        if (type == JSON_TYPE_PAIR_NAME) {
            char buffer[64];
            os_bzero(buffer, 64);
            if (jsonparse_strcmp_value(parser, "led") == 0) {
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, buffer, 64);

                if (!os_strcmp(buffer, "on")) {
                    led_on = 1;
                } else if (!os_strcmp(buffer, "off")) {
                    led_on = 0;
                } else if(!os_strcmp(buffer, "toggle")){
                    led_on = ~led_on & 0x01;
                }
            } else if (jsonparse_strcmp_value(parser, "display") == 0) {
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, buffer, 64);

                if (!os_strcmp(buffer, "on"))
                    show_display = true;
                else if (!os_strcmp(buffer, "off"))
                    show_display = false;
            } else if (jsonparse_strcmp_value(parser, "time_offset") == 0) {
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, buffer, 64);
                os_printf("time offset: %s\n", buffer);
                ntp_set_time_offset(atoi(buffer));
            } else if(jsonparse_strcmp_value(parser, "manual_temp") == 0){
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, buffer, 64);

                uint8_t i;
                bool startReplace = false;
                for(i = 0; i < os_strlen(buffer); i++){
                    if(buffer[i] == '.')
                        startReplace = true;
                    if(startReplace)
                        buffer[i] = buffer[i+1];
                }
                buffer[os_strlen(buffer)] = '\0';

                manual_temp = atoi(buffer);
            } else if(jsonparse_strcmp_value(parser, "weatherWoeid") == 0){
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, buffer, 64);
                weather_setWoeid(atoi(buffer));
                weather_refreshData();
            }
        }
    }
    return 0;
}

ICACHE_FLASH_ATTR
void json_parse_command(char *json) {
    struct jsontree_context js;
    jsontree_setup(&js, (struct jsontree_value *) &command_tree, json_putchar);
    json_parse(&js, json);
}

