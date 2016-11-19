
#include "ets_sys.h"
#include "mem.h"
#include "osapi.h"
#include "spi_flash.h"
#include "my_flash.h"

ICACHE_FLASH_ATTR
void my_flash_writeValue16(char *data, uint16_t value){
    data[0] = value & 0xff;
    data[1] = value >> 8;
}

ICACHE_FLASH_ATTR
uint16_t my_flash_readValue16(char *data){
    uint16_t value = 0;
    value = (data[0]) | (data[1] << 8);
    return value;
}

ICACHE_FLASH_ATTR
void my_flash_writeValue32(char *data, uint32_t value){
    data[0] = value & 0xff;
    data[1] = (value >> 8) & 0xff;
    data[2] = (value >> 16) & 0xff;
    data[3] = (value >> 24) & 0xff;
}

ICACHE_FLASH_ATTR
uint32_t my_flash_readValue32(char *data){
    uint32_t value = 0;
    value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    return value;
}

ICACHE_FLASH_ATTR
void my_flash_write(uint32_t sector, char *data, uint32_t len){
    //write len header
    char magic_data[] = {0xde, 0xad, 0xbe, 0xef, 0x00, 0x00, 0x00, 0x00};
    my_flash_writeValue32(magic_data + 4, len);

    spi_flash_erase_sector(sector);

    uint32_t dstAddress = sector * SPI_FLASH_SEC_SIZE;
    spi_flash_write(dstAddress, (uint32 *)magic_data, sizeof(magic_data));
    spi_flash_write(dstAddress + sizeof(magic_data), (uint32 *)data, len);
}

ICACHE_FLASH_ATTR
void my_flash_read(uint32_t sector, char **data, uint32_t *len){
    char magic_data[] = {0xde, 0xad, 0xbe, 0xef, 0x00, 0x00, 0x00, 0x00};

    //read len from last four bytes of magic_data (part of header)
    char flashData[8];
    uint32_t srcAddress = sector * SPI_FLASH_SEC_SIZE;
    spi_flash_read(srcAddress, (uint32 *)flashData, sizeof(flashData));

    if(os_memcmp(magic_data, flashData, 4) != 0){
        return;
    }

    *len = my_flash_readValue32(flashData + 4);

    char *readFlash = (char *)os_zalloc(sizeof(char) * (*len));
    spi_flash_read(srcAddress + sizeof(magic_data), (uint32 *)readFlash, *len);
    *data = readFlash;
}
