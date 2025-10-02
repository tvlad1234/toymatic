#include <stdint.h>

#include "libopencm3/stm32/gpio.h"

#include "utils.h"
#include "setup.h"
#include "pins.h"

#include "plc.h"
#include "modbus.h"
#include "mb_diag.h"
#include "blink.h"

uint8_t *plc_binary = (uint8_t *)PLC_BIN_BASE;

struct blink_ctx red_blinker = {
	.gpio = LED_RED_GPIO,
	.pin = LED_RED_PIN,
	.state = 1,
};

struct PLC plc = {
	.outs = {
		{.mcu_gpio = GPIOB, .mcu_pin = GPIO6},
		{.mcu_gpio = GPIOA, .mcu_pin = GPIO12},
		{.mcu_gpio = GPIOA, .mcu_pin = GPIO11},
		{.mcu_gpio = GPIOA, .mcu_pin = GPIO8},
	},

	.ins = {
		{.mcu_gpio = GPIOA, .mcu_pin = GPIO7},
		{.mcu_gpio = GPIOA, .mcu_pin = GPIO6},
		{.mcu_gpio = GPIOA, .mcu_pin = GPIO5},
		{.mcu_gpio = GPIOA, .mcu_pin = GPIO4},
	},
	.Cycle_time = 100000,
	.run = 1,
};

struct modbus_pdu mb_pdu;

int32_t max_duration = 0;

int main(void)
{
	rcc_setup();
	systick_setup();
	usart_setup();
	led_setup();

	plc_init_io(&plc);

	if (validate_program(plc_binary))
		plc.fault_code = PLC_ERR_BIN_HEADER;
	else
		plc.fault_code = plc_init_program(&plc, plc_binary + 8);

	while (1)
	{
		uint32_t loop_start = micros();

		if (plc.fault_code == PLC_OK)
		{
			if (plc.run && !gpio_get(RUN_SW_GPIO, RUN_SW_PIN))
			{
				// PLC cycle
				gpio_set(LED_GREEN_GPIO, LED_GREEN_PIN);
				gpio_clear(LED_RED_GPIO, LED_RED_PIN);
				if ((int32_t)(loop_start - plc.next_cycle) >= 0)
				{
					// Read inputs
					plc_read_inputs(&plc);

					// Execute program
					plc.fault_code = plc_interpret_cycle(&plc);

					// Update outputs
					plc_update_outputs(&plc);

					plc.next_cycle += plc.Cycle_time;
				}
			}
			else
			{
				gpio_clear(LED_GREEN_GPIO, LED_GREEN_PIN);
				gpio_set(LED_RED_GPIO, LED_RED_PIN);
			}
		}
		else
		{
			gpio_clear(LED_GREEN_GPIO, LED_GREEN_PIN);
			red_blinker.target = plc.fault_code;
			blink_update(&red_blinker, loop_start);
		}

		uint32_t now = micros();

		// Check whether Modbus is active
		modbus_check_rx(&mb, now);

		// Query diagnostic server every 500ms
		diag_client(&plc, &mb, &mb_pdu, now);

		// Verify that cycle timing is met
		int32_t loop_duration = (int32_t)(micros() - loop_start);
		if (loop_duration > max_duration)
			max_duration = loop_duration;

		if (loop_duration > (int32_t)plc.Cycle_time)
			plc.fault_code = PLC_ERR_CYCLE_TIME;
	}
}
