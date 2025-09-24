#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "libopencm3/stm32/gpio.h"

#include "utils.h"

#include "plc.h"
#include "vm.h"
#include "crc16.h"

void plc_init_io(struct PLC *plc)
{
    // Setup inputs and outputs
    for (int i = 0; i < 4; i++)
    {
        // Outputs
        gpio_mode_setup(plc->outs[i].mcu_gpio, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, plc->outs[i].mcu_pin);
        gpio_clear(plc->outs[i].mcu_gpio, plc->outs[i].mcu_pin);

        // Inputs
        gpio_mode_setup(plc->ins[i].mcu_gpio, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, plc->ins[i].mcu_pin);
    }
}

int plc_init_program(struct PLC *plc, uint8_t *bin)
{
    plc->program = bin;
    memset(plc->ram, 0, PLC_RAM_SIZE);
    memset(&plc->core, 0, sizeof(struct MiniRV32IMAState));
    plc->core.pc = MINIRV32_RAM_IMAGE_OFFSET;
    plc->core.extraflags |= 3; // Machine-mode.
    plc->core.plc_cycle_complete = 1;

    uint32_t cycle_start = micros();

    while (!plc->core.plc_program_init)
    {
        uint32_t now_us = micros();

        vm_flip(plc, now_us);
        if ((int32_t)(now_us - cycle_start) >= (int32_t)100000) // 100ms timeout
            return PLC_ERR_VM_TIMEOUT;
    }

    plc->Cycle_time = plc->core.plc_out;
    plc->next_cycle = micros();

    return PLC_OK;
}

int plc_interpret_cycle(struct PLC *plc)
{
    plc->core.plc_cycle_complete = 0;

    uint32_t cycle_start = micros();

    while (!plc->core.plc_cycle_complete)
    {
        uint32_t now_us = micros();

        vm_flip(plc, now_us);
        if ((int32_t)(now_us - cycle_start) >= (int32_t)plc->Cycle_time)
            return PLC_ERR_VM_TIMEOUT;
    }

    return PLC_OK;
}

void plc_read_inputs(struct PLC *plc)
{
    uint8_t ins = 0;
    for (int i = NUM_GPI - 1; i >= 0; i--)
    {
        if (gpio_get(plc->ins[i].mcu_gpio, plc->ins[i].mcu_pin))
            ins = (ins << 1) | 1;
        else
            ins = ins << 1;
    }
    plc->core.plc_in = ins;
}

void plc_update_outputs(struct PLC *plc)
{
    uint8_t outs = plc->core.plc_out;

    for (int i = 0; i < NUM_GPO; i++)
    {
        if ((outs >> i) & 1)
            gpio_set(plc->outs[i].mcu_gpio, plc->outs[i].mcu_pin);
        else
            gpio_clear(plc->outs[i].mcu_gpio, plc->outs[i].mcu_pin);
    }
}
