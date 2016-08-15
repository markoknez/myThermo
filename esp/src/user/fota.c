#include "ets_sys.h"
#include "osapi.h"
#include "upgrade.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"

ICACHE_FLASH_ATTR static void ota_finished_callback(void *arg);
ICACHE_FLASH_ATTR void handleUpgrade(uint8_t serverVersion, const char *server_ip, uint16_t port, const char *path);

#define requestHeader "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: ESP8266\r\n\
Accept: */*\r\n\
Content-Type: application/json\r\n"

void ota_finished_callback(void *arg) {
    struct upgrade_server_info *update = arg;
    if (update->upgrade_flag == true) {
        os_printf("[OTA]success; rebooting!\n");
        system_upgrade_reboot();
    } else {
        os_printf("[OTA]failed!\n");
    }

    os_free(update->pespconn);
    os_free(update->url);
    os_free(update);
}

void handleUpgrade(uint8_t serverVersion, const char *server_ip, uint16_t port, const char *path) {
    const char* file;
    uint8_t userBin = system_upgrade_userbin_check();
    switch (userBin) {
        case UPGRADE_FW_BIN1:
            file = "user2.bin";
            break;
        case UPGRADE_FW_BIN2:
            file = "user1.bin";
            break;
        default:
            os_printf("[OTA]Invalid userbin number!\n");
            return;
    }

    uint16_t version = 1;
    if (serverVersion <= version) {
        os_printf("[OTA]No update. Server version:%d, local version %d\n", serverVersion, version);
        return;
    }

    os_printf("[OTA]Upgrade available version: %d\n", serverVersion);

    struct upgrade_server_info* update = (struct upgrade_server_info *) os_zalloc(sizeof(struct upgrade_server_info));
    update->pespconn = (struct espconn *) os_zalloc(sizeof(struct espconn));

    os_memcpy(update->ip, server_ip, 4);
    update->port = port;

    os_printf("[OTA]Server "IPSTR":%d. Path: %s%s\n", IP2STR(update->ip), update->port, path, file);
    os_sprintf(update->pre_version, "v%d.%d", 1, 0);
    os_sprintf(update->upgrade_version, "v%d.%d", 1, 1);

    update->check_cb = ota_finished_callback;
    update->check_times = 100000;
    update->url = (uint8 *) os_zalloc(512);

    os_sprintf((char*)update->url,
            "GET %s%s HTTP/1.1\r\n"\
            "Host: "IPSTR":%d\r\n"\
            ""requestHeader"\r\n"\
            "\r\n",
            path, file, IP2STR(update->ip), update->port);

    if (system_upgrade_start(update) == false) {
        os_printf("[OTA]Could not start upgrade\n");

        os_free(update->pespconn);
        os_free(update->url);
        os_free(update);
    } else {
        os_printf("[OTA]Upgrading...\n");
    }
}
