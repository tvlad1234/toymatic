#include <stdio.h>
#include <stdint.h>

#include "config.c"
#include "resource1.c"

extern volatile uint32_t TIMERL;
extern volatile uint32_t PLC_IN;
extern volatile uint32_t PLC_OUT;
extern volatile uint32_t PLC_INIT;
extern volatile uint32_t PLC_END;

TIME __CURRENT_TIME;

// Update __CURRENT_TIME for timers
static void update_time(void)
{
	__CURRENT_TIME.tv_sec = TIMERL / 1000000;
	__CURRENT_TIME.tv_nsec = (TIMERL % 1000000) * 1000;
}

uint32_t ins = 0;
uint32_t outs = 0;

int main()
{
	PLC_OUT = common_ticktime__ / 1000; // VM will retrieve cycle time from PLC_OUT during init
	config_init__();
	PLC_INIT = 1;

	while (1)
	{
		// Wait for next cycle
		while (PLC_END)
			;

		//  Scan inputs
		ins = PLC_IN;
		GLOBAL__X1->value = ins & 1;
		GLOBAL__X2->value = ins & 2;
		GLOBAL__X3->value = ins & 4;
		GLOBAL__X4->value = ins & 8;

		// Run program
		update_time();
		RESOURCE1_run__(1);

		// Update outputs
		outs = GLOBAL__Y4->value;
		outs = (outs << 1) | GLOBAL__Y3->value;
		outs = (outs << 1) | GLOBAL__Y2->value;
		outs = (outs << 1) | GLOBAL__Y1->value;

		PLC_OUT = outs;

		// Signal end of cycle
		PLC_END = 1;
	}

	return 0;
}
