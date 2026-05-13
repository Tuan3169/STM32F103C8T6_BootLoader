ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08004000, LENGTH = 23K
    USER_DATA (rx) : ORIGIN = 0x08009C00, LENGTH = 1K
    RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 20K
}

_estack = ORIGIN(RAM) + LENGTH(RAM) - 128;

SECTIONS
{
    .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
    .text : { *(.text*) *(.rodata*) } > FLASH

    .user_data :
    {
        _suser_data = .;
        KEEP(*(.user_data))
        _euser_data = .;
    } > USER_DATA

    .user_crc (NOLOAD) :
    {
        . = ORIGIN(USER_DATA) + LENGTH(USER_DATA) - 8;
        KEEP(*(.user_crc))
    } > USER_DATA

    .user_hash (NOLOAD) :
    {
        . = ORIGIN(USER_DATA) + LENGTH(USER_DATA) - 4;
        KEEP(*(.user_hash))
    } > USER_DATA

    .data : 
    {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH

    _sidata = LOADADDR(.data);

    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    .boot_flag (NOLOAD) :
    {
        *(.boot_flag)
    } > RAM AT > RAM
}