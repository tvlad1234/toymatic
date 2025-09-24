#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <termios.h>

int rs485_open(const char *dev, speed_t baudrate);
void rs485_close(void);
void rs485_write(uint8_t *buf, int len);
int rs485_read_byte(uint8_t *b);

void utils_init(void);   // must be called once at program start
uint32_t micros(void);

#endif
