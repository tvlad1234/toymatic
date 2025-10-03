#include <stdint.h>
#include "modbus.h"

void modbus_func_pdu_write_multiple_coils(struct modbus_pdu *mb_pdu, uint16_t start_addr, uint16_t num_coils, const uint8_t *coil_values)
{
    mb_pdu->len = 0;

    mb_pdu->data[mb_pdu->len++] = 0x0F;              // write multiple coils
    mb_pdu->data[mb_pdu->len++] = start_addr >> 8;   // addr hi
    mb_pdu->data[mb_pdu->len++] = start_addr & 0xFF; // addr lo
    mb_pdu->data[mb_pdu->len++] = num_coils >> 8;    // quantity hi
    mb_pdu->data[mb_pdu->len++] = num_coils & 0xFF;  // quantity lo

    uint8_t byte_count = (num_coils + 7) / 8; // compute byte count
    mb_pdu->data[mb_pdu->len++] = byte_count; // byte count

    for (uint8_t i = 0; i < byte_count; i++)
        mb_pdu->data[mb_pdu->len++] = coil_values[i];
}

void modbus_func_pdu_read_discrete_inputs(struct modbus_pdu *mb_pdu, uint16_t start_addr, uint16_t num_contacts)
{
    mb_pdu->len = 0;

    mb_pdu->data[mb_pdu->len++] = 0x02;                // read discrete inputs
    mb_pdu->data[mb_pdu->len++] = start_addr >> 8;     // addr hi
    mb_pdu->data[mb_pdu->len++] = start_addr & 0xFF;   // addr lo
    mb_pdu->data[mb_pdu->len++] = num_contacts >> 8;   // quantity hi
    mb_pdu->data[mb_pdu->len++] = num_contacts & 0xFF; // quantity lo
}
