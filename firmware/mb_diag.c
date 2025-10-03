#include "utils.h"
#include "plc.h"
#include "mb_diag.h"

void query_diag_server(struct PLC *plc, struct modbus_ctx *mb, struct modbus_pdu *mb_pdu)
{
    mb_pdu->data[0] = DIAG_SERVER_QUERY;
    mb_pdu->data[1] = plc->fault_code;
    mb_pdu->len = 2;

    modbus_send_pdu(mb, DIAG_SERVER_ADDR, mb_pdu);
}

void handle_diag_response(struct PLC *plc, struct modbus_ctx *mb, struct modbus_pdu *mb_pdu)
{
    // Handle response from diag server
    int res = modbus_process_adu(mb, DIAG_SERVER_ADDR, mb_pdu);
    if (res == MB_OK)
    {
        uint8_t cmd = mb_pdu->data[0];

        if (cmd == CMD_RUN) // RUN
            plc->run = 1;
        else if (cmd == CMD_STOP) // STOP
            plc->run = 0;
        else if (cmd == CMD_CLEAR_FAULT)
            plc->fault_code = PLC_OK;

        else if (cmd == CMD_FLASH_PREPARE)
        {
            // Construct header
            uint8_t prog_header[8];
            prog_header[0] = 'P';
            prog_header[1] = 'R';
            prog_header[2] = 'O';
            prog_header[3] = 'G';
            prog_header[4] = mb_pdu->data[1];
            prog_header[5] = mb_pdu->data[2];
            prog_header[6] = mb_pdu->data[3];
            prog_header[7] = mb_pdu->data[4];

            // Unlock and erase flash
            flash_unlock();
            flash_erase_page(PLC_BIN_BASE / FLASH_PAGE_SIZE);
            flash_erase_page((PLC_BIN_BASE / FLASH_PAGE_SIZE) + 1);
            flash_program_double_word(PLC_BIN_BASE, *((uint64_t *)prog_header));
        }

        else if (cmd == CMD_FLASH_WRITE)
        {
            uint16_t num_chunks = mb_pdu->data[1] | (mb_pdu->data[2] << 8);
            uint16_t off = (mb_pdu->data[3] | (mb_pdu->data[4] << 8)) + 8;

            uint8_t chunk_data[8];
            uint8_t chunk_byte = 5;
            for (int i = 0; i < num_chunks; i++)
            {
                for (int j = 0; j < 8; j++)
                    chunk_data[j] = mb_pdu->data[chunk_byte++];
                flash_program_double_word(PLC_BIN_BASE + off, *((uint64_t *)chunk_data));
                off += 8;
            }
        }

        else if (cmd == CMD_FLASH_VALIDATE)
        {
            flash_lock();
            if (validate_program((uint8_t *)PLC_BIN_BASE))
                plc->fault_code = PLC_ERR_BIN_HEADER;
            else
                plc->fault_code = plc_init_program(plc, (uint8_t *)PLC_BIN_BASE + 8);
        }
    }
    mb->state = MB_IDLE;
}
