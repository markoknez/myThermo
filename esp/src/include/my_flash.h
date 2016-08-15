/*
 * my_flash.h
 *
 *  Created on: 8. ožu 2016.
 *      Author: Marko
 */

#ifndef SRC_INCLUDE_MY_FLASH_H_
#define SRC_INCLUDE_MY_FLASH_H_


void my_flash_writeValue16(char *data, uint16_t value);
uint16_t my_flash_readValue16(char *data);

void my_flash_writeValue32(char *data, uint32_t value);
uint32_t my_flash_readValue32(char *data);

void my_flash_write(uint32_t sector, char *data, uint32_t len);
void my_flash_read(uint32_t sector, char **data, uint32_t *len);

#endif /* SRC_INCLUDE_MY_FLASH_H_ */
