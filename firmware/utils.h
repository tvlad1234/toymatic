#ifndef _UTILS_H
#define _UTILS_H

#include "modbus.h"

#include "libopencm3/stm32/flash.h"

#define FLASH_SIZE (32 * 1024)
#define FLASH_PAGE_SIZE 2048
#define FLASH_END (FLASH_BASE + FLASH_SIZE)

#define PLC_BIN_SIZE (2 * FLASH_PAGE_SIZE)
#define PLC_BIN_BASE (FLASH_END - PLC_BIN_SIZE)

void delay_ms(int ms);
void delay_us(int us);
uint32_t micros(void);

int validate_program(uint8_t *bin);
void rs485_write(uint8_t *data, unsigned int len);

extern volatile struct modbus_ctx mb;

#endif
