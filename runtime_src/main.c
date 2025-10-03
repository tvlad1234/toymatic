#include <stdio.h>
#include <stdint.h>

#include "config.c"
#include "resource1.c"

#define ARRAY_SIZE(foo) (sizeof(foo) / sizeof(foo[0]))

extern volatile uint32_t TIMERL;
extern volatile uint32_t PLC_IN;
extern volatile uint32_t PLC_OUT;
extern volatile uint32_t PLC_INIT;
extern volatile uint32_t PLC_END;
extern volatile uint32_t HMI_NUM;
extern volatile uint32_t HMI_COILS;
extern volatile uint32_t HMI_CONTACTS;

TIME __CURRENT_TIME;

// Update __CURRENT_TIME for timers
static void update_time(void)
{
	__CURRENT_TIME.tv_sec = TIMERL / 1000000;
	__CURRENT_TIME.tv_nsec = (TIMERL % 1000000) * 1000;
}

uint32_t data;
int i;

int main()
{
	PLC_OUT = common_ticktime__ / 1000; // VM will retrieve cycle time from PLC_OUT during init
	config_init__();

	HMI_NUM = (ARRAY_SIZE(GLOBAL__HMI_REGS->value.table) << 16) | (ARRAY_SIZE(GLOBAL__HMI_COILS->value.table) << 8) | ARRAY_SIZE(GLOBAL__HMI_CONTACTS->value.table);

	PLC_INIT = 1;

	while (1)
	{
		// Wait for next cycle
		while (PLC_END)
			;

		//  Scan inputs
		data = PLC_IN;
		for (i = 0; i < ARRAY_SIZE(GLOBAL__IX->value.table); i++)
			GLOBAL__IX->value.table[i] = data & (1 << i);

		// Scan HMI inputs
		data = HMI_CONTACTS;
		for (i = 0; i < ARRAY_SIZE(GLOBAL__HMI_CONTACTS->value.table); i++)
			GLOBAL__HMI_CONTACTS->value.table[i] = data & (1 << i);

		// Run program
		update_time();
		RESOURCE1_run__(1);

		// Update outputs
		data = 0;
		for (i = ARRAY_SIZE(GLOBAL__QX->value.table) - 1; i >= 0; i--)
			if (GLOBAL__QX->value.table[i])
				data |= (1U << i);
		PLC_OUT = data;

		// Update HMI discrete outputs
		data = 0;
		for (i = ARRAY_SIZE(GLOBAL__HMI_COILS->value.table) - 1; i >= 0; i--)
			if (GLOBAL__HMI_COILS->value.table[i])
				data |= (1U << i);
		HMI_COILS = data;

		// Signal end of cycle
		PLC_END = 1;
	}

	return 0;
}
