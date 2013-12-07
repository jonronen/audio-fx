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
    .dramcopystart (NOLOAD) :
    {
        _dramcopystart = .;
    } > DRAM
    
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

    .dramcopyend (NOLOAD) :
    {
        _dramcopyend = .;
    } > DRAM

    .itext :
    {
        _iramstart = .; // always 0
        *(.vectors)
        KEEP(*(.vectors));// otherwise there are no references to it and the linker strip it
        _iramend = .;
    } > IRAM AT> DRAM

    _iramcopy = LOADADDR(.itext);

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
        . += 0x400;
        irq_stack = .;
        . += 0x400;
        fiq_stack = .;
        . += 0x400;
    } > DRAM

    /* physical address of the stack */
    /*stackend_phys = stackend - CACHED_DRAM_ADDR + UNCACHED_DRAM_ADDR;*/

    /* treat .bss and .ncbss as a single section */
    .bss (NOLOAD) :
    {
        _bss_start = .;
        *(.bss*);
    } > DRAM

    /* align on cache size boundary to avoid mixing cached and noncached stuff */
    .ncbss . - CACHED_DRAM_ADDR + UNCACHED_DRAM_ADDR (NOLOAD) :
    {
        . = ALIGN(CACHEALIGN_SIZE);
        *(.ncbss*)
        . = ALIGN(CACHEALIGN_SIZE);
    } AT> DRAM

    .bssendadr (NOLOAD) :
    {
        _bss_end = .;
    } > DRAM
}
