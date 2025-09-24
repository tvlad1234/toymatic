#ifndef _PLC_COMMANDS_H
#define _PLC_COMANDS_H

#include <stdint.h>
#include "modbus.h"

#define DIAG_SERVER_ADDR 247

enum PLC_COMMANDS
{
    DIAG_SERVER_QUERY = 65,
    CMD_RUN,
    CMD_STOP,
    CMD_FLASH_PREPARE,
    CMD_FLASH_WRITE,
    CMD_FLASH_VALIDATE,
    CMD_CLEAR_FAULT,
};

enum plc_error
{
    PLC_OK = 0,
    PLC_ERR_BIN_HEADER,
    PLC_ERR_VM_TIMEOUT,
    PLC_ERR_CYCLE_TIME,
    PLC_NUM_ERR,
    PLC_COMMS_TIMEOUT,
    UPLOAD_FILE_NOT_FOUND,
    UPLOAD_FILE_TOO_LARGE,
    UPLOAD_FILE_ERR,
};

#define PLC_run(mb) PLC_single_command(mb, CMD_RUN)
#define PLC_stop(mb) PLC_single_command(mb, CMD_STOP)
#define PLC_clear_fault(mb) PLC_single_command(mb, CMD_CLEAR_FAULT)
#define PLC_validate_flash(mb) PLC_single_command(mb, CMD_FLASH_VALIDATE)

int get_plc_status(struct modbus_ctx *ctx);
const char* PLC_err_msg(int err);

int PLC_single_command(struct modbus_ctx *mb, uint8_t cmd);
int PLC_prepare_flash(struct modbus_ctx *mb, uint16_t len, uint16_t crc);
int PLC_write_flash(struct modbus_ctx *mb, uint16_t num_chunks, uint16_t off, uint8_t *data);
int PLC_erase(struct modbus_ctx *mb);
int PLC_upload(struct modbus_ctx *mb, char *filename);

#endif