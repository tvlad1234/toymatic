#include "libopencm3/cm3/systick.h"
#include "libopencm3/cm3/nvic.h"
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/stm32/spi.h"
#include "libopencm3/stm32/usart.h"

#include "pins.h"
#include "setup.h"

void rcc_setup(void)
{
    rcc_clock_setup(&rcc_clock_config[RCC_CLOCK_CONFIG_HSI_PLL_64MHZ]);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);

    rcc_periph_clock_enable(RCC_USART2);
    rcc_periph_reset_pulse(RST_USART2);
}

void systick_setup(void)
{
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_clear();
    systick_set_reload(rcc_ahb_frequency / 10000 + 1);
    systick_interrupt_enable();
    systick_counter_enable();
}

void led_setup(void)
{
    gpio_mode_setup(RUN_SW_GPIO, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, RUN_SW_PIN);

    gpio_mode_setup(LED_GREEN_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_GREEN_PIN);
    gpio_mode_setup(LED_RED_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_RED_PIN);

    gpio_clear(LED_GREEN_GPIO, LED_GREEN_PIN);
    gpio_clear(LED_RED_GPIO, LED_RED_PIN);
}

void usart_setup(void)
{
    gpio_mode_setup(RS485_TXEN_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RS485_TXEN_PIN);
    gpio_clear(RS485_TXEN_GPIO, RS485_TXEN_PIN); // Rx

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);   // Tx
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO3); // Rx, pullup
    gpio_set_af(GPIOA, GPIO_AF1, GPIO2 | GPIO3);

    usart_set_baudrate(USART2, 115200);
    usart_set_databits(USART2, 8);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

    usart_enable_rx_interrupt(USART2);
    nvic_enable_irq(NVIC_USART2_LPUART2_IRQ);

    usart_enable(USART2);
}
