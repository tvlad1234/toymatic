#include "libopencm3/stm32/spi.h"
#include "libopencm3/stm32/usart.h"
#include "libopencm3/cm3/nvic.h"
#include "libopencm3/stm32/gpio.h"

#include <string.h>

#include "utils.h"
#include "pins.h"

#include "modbus.h"

volatile uint32_t _micros;

volatile struct modbus_ctx mb = {
    .byte_period = 87, // for 115200 baud
};

// Systick timer interrupt; every 100us
void sys_tick_handler(void)
{
    _micros += 100;
}

// UART interupt
void usart2_lpuart2_isr(void)
{
    modbus_receive_byte(&mb, usart_recv(USART2), _micros);
}

// Microseconds since boot
uint32_t micros(void)
{
    return _micros;
}

void delay_ms(int ms)
{
    uint32_t target = _micros + (ms * 1000);
    while (_micros < target)
        ;
}

void delay_us(int us)
{
    uint32_t target = _micros + us;
    while (_micros < target)
        ;
}

void rs485_write(uint8_t *data, unsigned int len)
{
    gpio_set(RS485_TXEN_GPIO, RS485_TXEN_PIN); // Tx
    while (len--)
        usart_send_blocking(USART2, *(data++));
    while (!usart_get_flag(USART2, USART_ISR_TC))
        ;
    gpio_clear(RS485_TXEN_GPIO, RS485_TXEN_PIN); // Rx
}

int validate_program(uint8_t *bin)
{
    /*
    Program header:
    - "PROG" (4 bytes)
    - Program size (2 bytes)
    - CRC16 (2 bytes)
*/

    // Check binary header
    if (*(bin++) != 'P' || *(bin++) != 'R' || *(bin++) != 'O' || *(bin++) != 'G')
        return 1;

    uint16_t len = *(bin++) | (*(bin++) << 8);
    uint16_t good_crc = *(bin++) | (*(bin++) << 8);
    uint16_t flash_crc = CRC16(bin, len);

    if (flash_crc != good_crc)
        return 1;

    return 0;
}
