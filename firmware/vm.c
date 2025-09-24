#include "vm.h"
#include "plc.h"

static uint32_t HandleException(uint32_t ir, uint32_t retval);
static uint32_t HandleControlStore(void *state, uint32_t addy, uint32_t val);
static uint32_t HandleControlLoad(void *state, uint32_t addy);

// This is the functionality we want to override in the emulator.
//  think of this as the way the emulator's processor is connected to the outside world.
#define MINIRV32WARN(x...) printf(x);
#define MINIRV32_DECORATE static
#define MINI_RV32_RAM_SIZE (PLC_RAM_SIZE + PLC_FLASH_SIZE)
#define MINIRV32_IMPLEMENTATION
#define MINIRV32_POSTEXEC(pc, ir, retval)         \
	{                                             \
		if (retval > 0)                           \
			retval = HandleException(ir, retval); \
	}
#define MINIRV32_HANDLE_MEM_STORE_CONTROL(addy, val) \
	if (HandleControlStore(state, addy, val))        \
		return val;
#define MINIRV32_HANDLE_MEM_LOAD_CONTROL(addy, rval) rval = HandleControlLoad(state, addy);
#define MINIRV32_OTHERCSR_WRITE(csrno, value) ;
#define MINIRV32_OTHERCSR_READ(csrno, value) value = 0;

#define MINIRV32_CUSTOM_MEMORY_BUS

#define MINIRV32_STORE4(ofs, val)                                                     \
	{                                                                                 \
		if (ofs >= PLC_FLASH_SIZE)                                                    \
			*(uint32_t *)((((struct PLC *)image)->ram) + ofs - PLC_FLASH_SIZE) = val; \
	}

#define MINIRV32_STORE2(ofs, val)                                                     \
	{                                                                                 \
		if (ofs >= PLC_FLASH_SIZE)                                                    \
			*(uint16_t *)((((struct PLC *)image)->ram) + ofs - PLC_FLASH_SIZE) = val; \
	}

#define MINIRV32_STORE1(ofs, val)                                                    \
	{                                                                                \
		if (ofs >= PLC_FLASH_SIZE)                                                   \
			*(uint8_t *)((((struct PLC *)image)->ram) + ofs - PLC_FLASH_SIZE) = val; \
	}

#define MINIRV32_LOAD4(ofs) ((ofs >= PLC_FLASH_SIZE) ? *(uint32_t *)((((struct PLC *)image)->ram) + ofs - PLC_FLASH_SIZE) : *(uint32_t *)((((struct PLC *)image)->program) + ofs))
#define MINIRV32_LOAD2(ofs) ((ofs >= PLC_FLASH_SIZE) ? *(uint16_t *)((((struct PLC *)image)->ram) + ofs - PLC_FLASH_SIZE) : *(uint16_t *)((((struct PLC *)image)->program) + ofs))
#define MINIRV32_LOAD1(ofs) ((ofs >= PLC_FLASH_SIZE) ? *(uint8_t *)((((struct PLC *)image)->ram) + ofs - PLC_FLASH_SIZE) : *(uint8_t *)((((struct PLC *)image)->program) + ofs))
#define MINIRV32_LOAD2_SIGNED(ofs) ((ofs >= PLC_FLASH_SIZE) ? *(int16_t *)((((struct PLC *)image)->ram) + ofs - PLC_FLASH_SIZE) : *(int16_t *)((((struct PLC *)image)->program) + ofs))
#define MINIRV32_LOAD1_SIGNED(ofs) ((ofs >= PLC_FLASH_SIZE) ? *(int8_t *)((((struct PLC *)image)->ram) + ofs - PLC_FLASH_SIZE) : *(int8_t *)((((struct PLC *)image)->program) + ofs))

#include "mini-rv32ima.h"

void vm_flip(struct PLC *plc, uint32_t time_us)
{
	uint32_t elapsedUs = time_us - plc->core.lastTime;
	plc->core.lastTime += elapsedUs;
	int ret = MiniRV32IMAStep(&plc->core, plc, 0, elapsedUs, 256);
}

static uint32_t HandleException(uint32_t ir, uint32_t code)
{
	// Weird opcode emitted by duktape on exit.
	if (code == 3)
	{
		// Could handle other opcodes here.
	}
	return code;
}

static uint32_t HandleControlStore(void *state, uint32_t addy, uint32_t val)
{

	struct MiniRV32IMAState *core = (struct MiniRV32IMAState *)state;

	if (addy == 0x11004004) // CLNT
		core->timermatchh = val;
	else if (addy == 0x11004000) // CLNT
		core->timermatchl = val;
	else if (addy == 0x11100000) // SYSCON (reboot, poweroff, etc.)
	{
		core->pc = core->pc + 4;
		return val; // NOTE: PC will be PC of Syscon.
	}
	else if (addy == 0x1100c004) // plc out
	{
		core->plc_out = val;
		// printf("PLC out: %d\n", val);
	}
	else if (addy == 0x1100c008) // plc program init
		core->plc_program_init = val;
	else if (addy == 0x1100c00c) // plc end
		core->plc_cycle_complete = val;

	return 0;
}

static uint32_t HandleControlLoad(void *state, uint32_t addy)
{
	struct MiniRV32IMAState *core = (struct MiniRV32IMAState *)state;

	if (addy == 0x1100bffc) // https://chromitem-soc.readthedocs.io/en/latest/clint.html
		return core->timerh;
	else if (addy == 0x1100bff8)
		return core->timerl;
	else if (addy == 0x1100c000) // plc in
		return core->plc_in;
	else if (addy == 0x1100c004) // plc out
		return core->plc_out;
	else if (addy == 0x1100c008) // plc program init
		return core->plc_program_init;
	else if (addy == 0x1100c00c) // plc end
		return core->plc_cycle_complete;
	return 0;
}
