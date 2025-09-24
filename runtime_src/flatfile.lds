ENTRY(initial_jump)

MEMORY
{
    FLASH (rx) : ORIGIN = 0x80000000, LENGTH = 4K
    RAM   (xrw): ORIGIN = 0x80001000, LENGTH = 4K
}

/* Reserve 1536 bytes for stack */
_stack_size = 1536;

SECTIONS
{
    /* --- Flash Sections --- */
    .init :
    { 
        _sinit = .;
        . = ALIGN(16);
        KEEP(*(SORT_NONE(.init)))
        . = ALIGN(16);
        _einit = .;
    } >FLASH AT>FLASH

    .text :
    {
        . = ALIGN(16);
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata*)
        *(.gnu.linkonce.t.*)
        . = ALIGN(16);
    } >FLASH AT>FLASH 

    .fini :
    {
        KEEP(*(SORT_NONE(.fini)))
        . = ALIGN(16);
    } >FLASH AT>FLASH

    .preinit_array :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >FLASH AT>FLASH 

    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >FLASH AT>FLASH 

    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH AT>FLASH 

    .ctors :
    {
        KEEP (*crtbegin.o(.ctors))
        KEEP (*crtbegin?.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
    } >FLASH AT>FLASH 

    .dtors :
    {
        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
    } >FLASH AT>FLASH 

    /* --- Data Section --- */
    .data :
    {
        . = ALIGN(16);
        PROVIDE(_data_vma = .);       /* RAM destination */
        *(.gnu.linkonce.r.*)
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(16);
        PROVIDE(_edata = .);
    } >RAM AT>FLASH

    /* LMA of .data in FLASH */
    PROVIDE(_data_lma = LOADADDR(.data));

    /* RISC-V global pointer, adjust as needed */
    PROVIDE(__global_pointer$ = _edata - 0x800);

    /* --- BSS Section --- */
    .bss :
    {
        . = ALIGN(16);
        PROVIDE(_sbss = .);
        *(.sbss*)
        *(.gnu.linkonce.sb.*)
        *(.bss*)
        *(.gnu.linkonce.b.*)    
        *(COMMON*)
        . = ALIGN(16);
        PROVIDE(_ebss = .);

        /* Ensure bss + stack fits in RAM */
        ASSERT(_ebss + _stack_size <= ORIGIN(RAM) + LENGTH(RAM),
               "RAM overflow! Reduce data/bss or increase RAM.");
    } >RAM AT>FLASH

    /* --- Stack Section --- */
    .stack (COPY):
    {
        . = ALIGN(16);
        stack_start = .;
        . += _stack_size;
        stack_end = .;
    } >RAM

    /* --- C runtime symbols --- */
    PROVIDE(_end = _ebss);
    PROVIDE(end = .);
    PROVIDE(_eusrstack = stack_end);

    /* --- Optional hardware symbols --- */
    PROVIDE(SYSCON = 0x11100000);
    PROVIDE(UART = 0x10000000);
    PROVIDE(TIMERL = 0x1100bff8);
    PROVIDE(TIMERH = 0x1100bffc);

    PROVIDE(PLC_IN = 0x1100c000);
    PROVIDE(PLC_OUT = 0x1100c004);

    PROVIDE(PLC_INIT = 0x1100c008);
    PROVIDE(PLC_END = 0x1100c00C);
}
