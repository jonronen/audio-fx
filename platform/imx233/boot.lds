#include "../../include/platform/imx233/imx233.h"

ENTRY(start)
OUTPUT_FORMAT(elf32-littlearm)
OUTPUT_ARCH(arm)
/*STARTUP(src/crt0.o)*/

/* Leave a hole at the beginning of the RAM to load the firmware */
#define RAM_HOLE    1024 * 1024

/* Make a difference between virtual and physical address so that we can use
 * the resulting elf file with the elftosb tools which loads at the *physical*
 * address */

MEMORY
{
    DRAM : ORIGIN = BASE_ADDR, LENGTH = 1024*1024
    IRAM : ORIGIN = IRAM_ORIG, LENGTH = IRAM_SIZE
}

SECTIONS
{
    loadaddress = BASE_ADDR;
    _loadaddress = BASE_ADDR;
    loadaddressend = BASE_ADDR + RAM_HOLE;
    _loadaddressend = BASE_ADDR + RAM_HOLE;

    . = BASE_ADDR;

    .start :
    {
        *(.start*)
    }

    .text :
    {
        *(.text*)
        *(.data*)
        *(.rodata*)
    } > DRAM

    .ibss (NOLOAD) :
    {
        _ibss_start = .;
        *(.ibss*)
        _ibss_end = .;
    } > IRAM

    .got :
    {
       *(.got)
    } > DRAM

    .got.plt :
    {
       *(.got.plt)
    } > DRAM

    .init_array :
    {
       init_array_begin = .;
       *(.init_array)
       init_array_end = .;
    } > DRAM

    .stack (NOLOAD) :
    {
        *(.stack)
        stackbegin = .;
        . += 0x2000;
        stackend = .;
        . += 0x2000;
        irq_stack = .;
        . += 0x2000;
        fiq_stack = .;
        . += 0x2000;
    } > DRAM

    /* physical address of the stack */
    /*stackend_phys = stackend - CACHED_DRAM_ADDR + UNCACHED_DRAM_ADDR;*/

    /* treat .bss and .ncbss as a single section */
    .bss (NOLOAD) :
    {
        _bss_start = .;
        *(.bss*);
    } > DRAM

    .bssendadr (NOLOAD) :
    {
        _bss_end = .;
    } > DRAM
}
