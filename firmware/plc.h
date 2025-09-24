#ifndef _PLC_H
#define _PLC_H

#include <stdint.h>
#include <stdbool.h>

#include "mini-rv32ima_core.h"

#define PLC_RAM_SIZE (4 * 1024)
#define PLC_FLASH_SIZE (4 * 1024)

#define NUM_GPI 4
#define NUM_GPO 4

enum plc_error
{
    PLC_OK = 0,
    PLC_ERR_BIN_HEADER,
    PLC_ERR_VM_TIMEOUT,
    PLC_ERR_CYCLE_TIME,
    PLC_NUM_ERR,
};

struct PLC_GPIO
{
    uint32_t mcu_gpio;
    uint16_t mcu_pin;
};

struct PLC
{
    uint8_t *program;
    uint8_t ram[PLC_RAM_SIZE];
    struct MiniRV32IMAState core;

    uint8_t fault_code;
    uint8_t mb_err;
    uint8_t run;
    unsigned int Cycle_time; // in microseconds
    uint32_t next_cycle;
    struct PLC_GPIO outs[NUM_GPO];
    struct PLC_GPIO ins[NUM_GPI];
};

void plc_init_io(struct PLC *plc);
int plc_init_program(struct PLC *plc, uint8_t *bin);
void plc_read_inputs(struct PLC *plc);
int plc_interpret_cycle(struct PLC *plc);
void plc_update_outputs(struct PLC *plc);

#endif