#ifndef _MINI_RV32IMA_CORE_H
#define _MINI_RV32IMA_CORE_H

#define MINIRV32_RAM_IMAGE_OFFSET 0x80000000

// As a note: We quouple-ify these, because in HLSL, we will be operating with
// uint4's.  We are going to uint4 data to/from system RAM.
//
// We're going to try to keep the full processor state to 12 x uint4.
struct MiniRV32IMAState
{
    uint32_t regs[32];

    uint32_t pc;
    uint32_t mstatus;
    uint32_t cyclel;
    uint32_t cycleh;

    uint32_t timerl;
    uint32_t timerh;
    uint32_t timermatchl;
    uint32_t timermatchh;

    uint32_t mscratch;
    uint32_t mtvec;
    uint32_t mie;
    uint32_t mip;

    uint32_t mepc;
    uint32_t mtval;
    uint32_t mcause;

    // Note: only a few bits are used.  (Machine = 3, User = 0)
    // Bits 0..1 = privilege.
    // Bit 2 = WFI (Wait for interrupt)
    // Bit 3+ = Load/Store reservation LSBs.
    uint32_t extraflags;

    uint64_t lastTime;
    uint32_t plc_in;
    uint32_t plc_out;
    uint8_t plc_program_init;
    uint8_t plc_cycle_complete;

    uint8_t hmi_reg_num;
    uint8_t hmi_coil_num;
    uint8_t hmi_contact_num;

    uint32_t hmi_coils;
    uint32_t hmi_contacts;
};

#endif