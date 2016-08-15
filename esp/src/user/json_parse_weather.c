#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "user_json.h"
#include "json/jsontree.h"
#include "json/json.h"
#include "json_parse_weather.h"
#include "weather.h"

weather_response_t weather_response = { 255, 0, NULL };

static ICACHE_FLASH_ATTR int json_set(struct jsontree_context *js_ctx, struct jsonparse_state *parser);
static struct jsontree_callback condition_callback = JSONTREE_CALLBACK(NULL, json_set);

JSONTREE_OBJECT(condition_tree, JSONTREE_PAIR("code", &condition_callback), JSONTREE_PAIR("temp", &condition_callback),
                JSONTREE_PAIR("text", &condition_callback));

JSONTREE_OBJECT(item_tree, JSONTREE_PAIR("condition", &condition_tree));
JSONTREE_OBJECT(channel_tree, JSONTREE_PAIR("item", &item_tree));
JSONTREE_OBJECT(query_tree, JSONTREE_PAIR("channel", &channel_tree));
JSONTREE_OBJECT(root_tree, JSONTREE_PAIR("query", &query_tree));

ICACHE_FLASH_ATTR
int json_set(struct jsontree_context *js_ctx, struct jsonparse_state *parser) {
    char buffer[64];

    int type;
    while ((type = jsonparse_next(parser)) != 0) {
        if (type == JSON_TYPE_PAIR_NAME) {
            if (jsonparse_strcmp_value(parser, "code") == 0) {
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, buffer, sizeof(buffer));
                weather_response.code = atoi(buffer);
            } else if (jsonparse_strcmp_value(parser, "temp") == 0) {
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, buffer, sizeof(buffer));
                weather_response.temp = atoi(buffer);
            } else if (jsonparse_strcmp_value(parser, "text") == 0) {
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, buffer, sizeof(buffer));
                char *text = (char *)os_zalloc(os_strlen(buffer));
                os_strcpy(text, buffer);

                os_free(weather_response.text);
                weather_response.text = text;
            }
        }
    }
}

ICACHE_FLASH_ATTR
void json_parse_weather(char *json) {
    struct jsontree_context js;
    jsontree_setup(&js, (struct jsontree_value *) &root_tree, json_putchar);
    json_parse(&js, json);
}
