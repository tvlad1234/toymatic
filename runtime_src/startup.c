#include <stddef.h>

void initial_jump() __attribute__((naked)) __attribute((section(".init")));
void *memset(void *dest, int c, size_t n);
void baremain(void);
int main(void);

void *memset(void *dest, int c, size_t n)
{
	unsigned char *s = dest;
	for (; n; n--, s++)
		*s = c;
	return dest;
}

void initial_jump(void)
{
	asm volatile("\n\
.option push\n\
.option norelax\n\
	la gp, __global_pointer$\n\
.option pop\n\
	la sp, _eusrstack\n");

	// Careful: Use registers to prevent overwriting of self-data.
	// This clears out BSS.
	asm volatile(
		"	la a0, _sbss\n\
	la a1, _ebss\n\
	li a2, 0\n\
	bge a0, a1, 2f\n\
1:	sw a2, 0(a0)\n\
	addi a0, a0, 4\n\
	blt a0, a1, 1b\n\
2:"
		// This loads DATA from FLASH to RAM.
		"	la a0, _data_lma\n\
	la a1, _data_vma\n\
	la a2, _edata\n\
1:	beq a1, a2, 2f\n\
	lw a3, 0(a0)\n\
	sw a3, 0(a1)\n\
	addi a0, a0, 4\n\
	addi a1, a1, 4\n\
	bne a1, a2, 1b\n\
2:\n"
#ifdef CPLUSPLUS
		// Call __libc_init_array function
		"	call %0 \n\t"
		:
		: "i"(__libc_init_array)
		: "a0", "a1", "a2", "a3", "a4", "a5", "t0", "t1", "t2", "memory"
#else
		:
		:
		: "a0", "a1", "a2", "a3", "memory"
#endif
	);

	baremain();
}

void baremain(void)
{
	main();

	while (1)
	{
		;
	}
}
