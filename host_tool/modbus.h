#ifndef _MODBUS_H
#define _MODBUS_H

#include <stdint.h>

// ADU = 1 byte address + PDU + 2 bytes checksum
#define MB_ADU_LEN 256
#define MB_PDU_LEN (MB_ADU_LEN - 3)

enum modbus_state
{
    MB_IDLE,
    MB_RECEIVING,
    MB_TRANSMITTING,
    MB_PROCESSING,
    MB_AWAIT_RESPONSE,
};

enum modbus_err
{
    MB_OK = 0,
    MB_OTHER_ADDR,
    MB_CRC_ERR,
    MB_BUSY,
    MB_NO_DATA,
};

struct modbus_pdu
{
    uint8_t data[MB_PDU_LEN];
    unsigned int len;
};

struct modbus_ctx
{
    uint8_t state;

    unsigned int byte_period;
    uint32_t last_byte_time;

    uint8_t adu_buf[MB_ADU_LEN];
    unsigned int adu_len;
};

void modbus_check_rx(struct modbus_ctx *ctx, uint32_t time_us);
void modbus_receive_byte(struct modbus_ctx *ctx, uint8_t b, uint32_t time_us);
int modbus_process_adu(struct modbus_ctx *ctx, uint8_t addr, struct modbus_pdu *pdu);
int modbus_read_pdu(struct modbus_ctx *ctx, uint8_t addr, struct modbus_pdu *pdu);
int modbus_send_pdu(struct modbus_ctx *ctx, uint8_t addr, struct modbus_pdu *pdu);

#endif