#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "plc_commands.h"
#include "modbus.h"
#include "utils.h"
#include "crc16.h"

// wait up to 500ms for a PDU
int modbus_wait_pdu(struct modbus_ctx *ctx, uint8_t addr, struct modbus_pdu *pdu)
{
    uint32_t start_us = micros();
    while (1)
    {
        modbus_check_rx(ctx, micros());

        int ret = modbus_read_pdu(ctx, addr, pdu);
        if (ret == MB_OK || ret == MB_CRC_ERR || ret == MB_OTHER_ADDR)
            return ret;

        // Timeout check
        if ((micros() - start_us) >= 500000UL) // 500 ms
            return MB_NO_DATA;

        usleep(500);
    }
}

static const char plc_err_msg[PLC_NUM_ERR + 1][50] = {"no fault", "invalid program header", "runtime VM timeout", "cycle time violation", "unknown fault"};

int get_plc_status(struct modbus_ctx *ctx)
{
    struct modbus_pdu pdu;

    int res = modbus_wait_pdu(ctx, DIAG_SERVER_ADDR, &pdu);
    if (res == MB_OK)
        return pdu.data[1];
    else
        return PLC_COMMS_TIMEOUT;
}

const char* PLC_err_msg(int err)
{
    if(err > PLC_NUM_ERR)
        return plc_err_msg[PLC_NUM_ERR];
    return plc_err_msg[err];
}

int PLC_single_command(struct modbus_ctx *mb, uint8_t cmd)
{
    struct modbus_pdu pdu;
    int ret = get_plc_status(mb);

    if (ret == PLC_COMMS_TIMEOUT)
        return ret;

    pdu.data[0] = cmd;
    pdu.len = 1;
    modbus_send_pdu(mb, DIAG_SERVER_ADDR, &pdu);

    return 0;
}

int PLC_prepare_flash(struct modbus_ctx *mb, uint16_t len, uint16_t crc)
{
    uint8_t cmd = CMD_FLASH_PREPARE;

    struct modbus_pdu pdu;
    int ret = get_plc_status(mb);
    if (ret == PLC_COMMS_TIMEOUT)
        return ret;

    pdu.data[0] = cmd;
    pdu.data[1] = len & 0xFF;
    pdu.data[2] = len >> 8;
    pdu.data[3] = crc & 0xFF;
    pdu.data[4] = crc >> 8;

    pdu.len = 5;
    modbus_send_pdu(mb, DIAG_SERVER_ADDR, &pdu);

    return 0;
}

int PLC_write_flash(struct modbus_ctx *mb, uint16_t num_chunks, uint16_t off, uint8_t *data)
{
    uint8_t cmd = CMD_FLASH_WRITE;
    struct modbus_pdu pdu;

    pdu.data[0] = cmd;
    pdu.data[1] = num_chunks & 0xFF;
    pdu.data[2] = num_chunks >> 8;
    pdu.data[3] = off & 0xFF;
    pdu.data[4] = off >> 8;
    pdu.len = 5;

    int ret = get_plc_status(mb);
    if (ret == PLC_COMMS_TIMEOUT)
        return ret;

    for (int i = 0; i < num_chunks; i++)
        for (int j = 0; j < 8; j++)
            pdu.data[pdu.len++] = *(data++);

    modbus_send_pdu(mb, DIAG_SERVER_ADDR, &pdu);

    return 0;
}

int PLC_erase(struct modbus_ctx *mb)
{
    int ret = PLC_prepare_flash(mb, 20, 0xabab);
    if (ret == PLC_COMMS_TIMEOUT)
        return ret;
    ret = PLC_validate_flash(mb);
    if (ret == PLC_COMMS_TIMEOUT)
        return ret;
    return PLC_OK;
}

#define PLC_FLASH_SIZE 4096
#define PROG_HEADER_SIZE 8
#define MAX_PROG_SIZE (PLC_FLASH_SIZE - PROG_HEADER_SIZE)

int PLC_upload(struct modbus_ctx *mb, char *filename)
{
    uint8_t plc_flash[PLC_FLASH_SIZE];

    // Load program
    FILE *f = fopen(filename, "rb");
    if (!f || ferror(f))
    {
        fprintf(stderr, "Error: \"%s\" not found\n", filename);
        return UPLOAD_FILE_NOT_FOUND;
    }
    fseek(f, 0, SEEK_END);
    long flen = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (flen > MAX_PROG_SIZE)
    {
        fprintf(stderr, "Error: Could not fit program (%ld bytes) into %d\n", flen, MAX_PROG_SIZE);
        return UPLOAD_FILE_TOO_LARGE;
    }
    printf("Program is %ld bytes long\n", flen);

    if (fread(plc_flash, flen, 1, f) != 1)
    {
        fprintf(stderr, "Error: Could not load image.\n");
        return UPLOAD_FILE_ERR;
    }
    fclose(f);

    uint16_t crc = CRC16(plc_flash, flen);
    printf("CRC16: %04x\n", crc);

    int ret = PLC_stop(mb);
    if (ret == PLC_COMMS_TIMEOUT)
        return ret;

    ret = PLC_prepare_flash(mb, flen, crc);
    if (ret == PLC_COMMS_TIMEOUT)
        return ret;

    uint16_t num_chunks = (flen / 8) + 1;
    uint16_t off = 0;
    uint8_t *chunk_data = plc_flash;

    for (int i = 0; i < num_chunks; i += 2)
    {
        putchar('.');
        fflush(stdout);
        ret = PLC_write_flash(mb, 2, off, chunk_data);
        if (ret == PLC_COMMS_TIMEOUT)
            return ret;
        off += 16;
        chunk_data += 16;
    }
    printf("\n");

    ret = PLC_validate_flash(mb);
    if (ret == PLC_COMMS_TIMEOUT)
        return ret;

    ret = get_plc_status(mb);

    if (ret == PLC_ERR_BIN_HEADER || ret == PLC_COMMS_TIMEOUT)
        return ret;

    return PLC_OK;
}
