#ifndef _MB_DIAG_H
#define _MB_DIAG_H

#include "plc.h"
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

void query_diag_server(struct PLC *plc, struct modbus_ctx *mb, struct modbus_pdu *mb_pdu);
void handle_diag_response(struct PLC *plc, struct modbus_ctx *mb, struct modbus_pdu *mb_pdu);

#endif