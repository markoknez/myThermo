/*
 * fota.h
 *
 *  Created on: 16. lis 2015.
 *      Author: Marko
 */

#ifndef IOT_DEMO_INCLUDE_FOTA_H_
#define IOT_DEMO_INCLUDE_FOTA_H_

void ICACHE_FLASH_ATTR handleUpgrade(uint8_t serverVersion, const char *server_ip, uint16_t port, const char *path);

#endif /* IOT_DEMO_INCLUDE_FOTA_H_ */
