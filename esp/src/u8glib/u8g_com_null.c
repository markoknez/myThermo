/*

 u8g_com_null.c
 
 communication null device

 Universal 8bit Graphics Library
 
 Copyright (c) 2011, olikraus@gmail.com
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list
 of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or other
 materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 
 */

#include "u8g.h"
#include "osapi.h"
#include "c_types.h"
#include "i2c_master.h"

uint8_t ICACHE_FLASH_ATTR u8g_com_null_startWrite(uint8_t addr){
	uint8_t ack;

	i2c_master_start();
	i2c_master_writeByte(addr);
	if(i2c_master_getAck()){
		os_printf("addr not acked\n");
		return 1;
	}
	return 0;
}

uint8_t ICACHE_FLASH_ATTR u8g_com_null_writeByte(uint8_t *data, uint16_t len){
	uint8_t i;
	for(i = 0; i < len; i++){
		i2c_master_writeByte(data[i]);
		if(i2c_master_getAck()){
			os_printf("data item %d of %d not acked\n", i, len);
			return 1;
		}
	}
	return 0;
}

uint8_t control = 0;

uint8_t ICACHE_FLASH_ATTR u8g_com_null_fn(u8g_t *u8g, uint8_t msg,
		uint8_t arg_val, void *arg_ptr) {

	switch (msg) {
	case U8G_COM_MSG_INIT:
		i2c_master_gpio_init();
		break;
	case U8G_COM_MSG_STOP:
		break;
	case U8G_COM_MSG_CHIP_SELECT:
		/* arg_val contains the chip number, which should be enabled */
		break;
	case U8G_COM_MSG_ADDRESS:
		if(arg_val == 0)
			control = 0;
		else control = 0x40;
		break;
	case U8G_COM_MSG_WRITE_BYTE:
		u8g_com_null_startWrite(0x78);
		u8g_com_null_writeByte(&control, 1);
		u8g_com_null_writeByte(&arg_val, 1);
		i2c_master_stop();
		break;
	case U8G_COM_MSG_WRITE_SEQ:
	case U8G_COM_MSG_WRITE_SEQ_P:
		u8g_com_null_startWrite(0x78);
		u8g_com_null_writeByte(&control, 1);
		u8g_com_null_writeByte(arg_ptr, arg_val);
		i2c_master_stop();
		break;
	}
	return 1;
}
