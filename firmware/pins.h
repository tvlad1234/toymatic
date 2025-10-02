#ifndef _PINS_H
#define _PINS_H

#include "libopencm3/stm32/gpio.h"

#define RUN_SW_GPIO GPIOC
#define RUN_SW_PIN GPIO14

#define LED_GREEN_GPIO GPIOC
#define LED_GREEN_PIN GPIO15

#define LED_RED_GPIO GPIOB
#define LED_RED_PIN GPIO7

#define RS485_TXEN_GPIO GPIOA
#define RS485_TXEN_PIN GPIO1

#endif