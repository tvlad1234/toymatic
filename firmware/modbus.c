
#include <stdint.h>
#include "modbus.h"
#include "crc16.h"

#include "utils.h"

void modbus_check_rx(struct modbus_ctx *ctx, uint32_t time_us)
{
    if (ctx->state == MB_RECEIVING && time_us > ctx->last_byte_time + 3 * ctx->byte_period)
        ctx->state = MB_PROCESSING;
}

void modbus_receive_byte(struct modbus_ctx *ctx, uint8_t b, uint32_t time_us)
{
    if (ctx->state == MB_IDLE || ctx->state == MB_AWAIT_RESPONSE)
    {
        ctx->adu_len = 0;
        ctx->state = MB_RECEIVING;
    }

    if (ctx->state == MB_RECEIVING)
    {
        ctx->last_byte_time = time_us;

        if (ctx->adu_len >= MB_ADU_LEN) // ignore messages that are too long
        {
            ctx->adu_len = 0;
            ctx->state = MB_IDLE;
            return;
        }

        ctx->adu_buf[ctx->adu_len++] = b;
    }
}

int modbus_process_adu(struct modbus_ctx *ctx, uint8_t addr, struct modbus_pdu *pdu)
{
    // check whether current message is of interest
    if (ctx->adu_buf[0] != addr)
    {
        ctx->adu_len = 0;     // clear buffer
        ctx->state = MB_IDLE; // go back to idle
        return MB_OTHER_ADDR;
    }

    // verify checksum
    uint8_t ch_lo = ctx->adu_buf[ctx->adu_len - 2];
    uint8_t ch_hi = ctx->adu_buf[ctx->adu_len - 1];
    uint16_t rec_chksum = ch_lo | (ch_hi << 8);                   // received checksum
    uint16_t comp_chksum = CRC16(ctx->adu_buf, ctx->adu_len - 2); // computed checksum

    if (comp_chksum != rec_chksum)
    {
        ctx->adu_len = 0;     // clear buffer
        ctx->state = MB_IDLE; // go back to idle
        return MB_CRC_ERR;
    }

    for (int i = 1; i < ctx->adu_len - 2; i++)
    {
        pdu->data[i - 1] = ctx->adu_buf[i];
    }
    pdu->len = ctx->adu_len - 3;

    ctx->adu_len = 0;     // clear buffer
    ctx->state = MB_IDLE; // go back to idle

    return MB_OK;
}

int modbus_read_pdu(struct modbus_ctx *ctx, uint8_t addr, struct modbus_pdu *pdu)
{
    if (ctx->state == MB_PROCESSING)
        return modbus_process_adu(ctx, addr, pdu);
    if (ctx->state == MB_RECEIVING)
        return MB_BUSY;
    return MB_NO_DATA;
}

int modbus_send_pdu(struct modbus_ctx *ctx, uint8_t addr, struct modbus_pdu *pdu)
{
    if (ctx->state != MB_IDLE)
        return MB_BUSY;

    ctx->state = MB_TRANSMITTING;

    ctx->adu_buf[0] = addr;
    ctx->adu_len = pdu->len + 1;
    for (int i = 0; i < pdu->len; i++)
        ctx->adu_buf[i + 1] = pdu->data[i];

    // COMPUTE CRC HERE
    uint16_t checksum = CRC16(ctx->adu_buf, ctx->adu_len);

    ctx->adu_buf[ctx->adu_len++] = checksum & 0xFF;
    ctx->adu_buf[ctx->adu_len++] = checksum >> 8;

    rs485_write(ctx->adu_buf, ctx->adu_len);

    ctx->state = MB_IDLE;
    return MB_OK;
}

int modbus_wait_processing(struct modbus_ctx *mb, uint32_t timeout_ms)
{
    uint32_t start = micros();
    while (mb->state != MB_PROCESSING)
    {
        modbus_check_rx(mb, micros());

        // timeout check
        if ((micros() - start) >= (timeout_ms * 1000UL))
            return 1; // timed out
    }

    return 0;
}
