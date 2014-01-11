#include "platform/imx233/imx233.h"

.text
.global start
start:

    bl    clear_ibss
    bl    clear_bss

    /* Set up stack for IRQ mode */ 
    msr     cpsr_c, #0xd2
    ldr     sp, =irq_stack

    /* Set up stack for FIQ mode */ 
    msr     cpsr_c, #0xd1
    ldr     sp, =fiq_stack

    /* Let svc, abort and undefined modes use irq stack */
    msr     cpsr_c, #0xd3
    ldr     sp, =irq_stack
    msr     cpsr_c, #0xd7
    ldr     sp, =irq_stack
    msr     cpsr_c, #0xdb
    ldr     sp, =irq_stack

    /* Switch to sys mode */
    msr     cpsr_c, #0xdf

    /* Set up some stack and munge it with 0xdeadbeef */
    ldr     sp, =stackend
    ldr     r2, =stackbegin
    ldr     r3, =0xdeadbeef
1:
    cmp     sp, r2
    strhi   r3, [r2], #4
    bhi     1b

    /* call the c++ static constructors */
    ldr     r4, =init_array_end
    ldr     r5, =init_array_begin
2:
    cmp     r4, r5
    bhi     3f
    b       4f
3:
    ldr     r3, [r5], #4
    blx     r3
    b       2b
4:

    /* load from serial (if there's anything to load */
    bl load_from_serial

    /*
     * now that everything is in place, enable the caches
     * (the function says enable MMU, but we don't really need the MMU...
     */
    bl enable_mmu

    /* time to go maaaain! */
    bl fx_main

    .pool

/*
 * This function clears out the .ibss section.
 */
clear_ibss:
    ldr    r0, =_ibss_start
    ldr    r1, =_ibss_end
    mov    r2, #0
1:    str    r2, [r0], #4
    cmp    r0, r1
    blo    1b
    mov    pc, lr

/*
 * This function clears out the .bss section.
 */
clear_bss:
    ldr    r0, =_bss_start
    ldr    r1, =_bss_end
    mov    r2, #0
1:    str    r2, [r0], #4
    cmp    r0, r1
    blo    1b
    mov    pc, lr

    .pool
/*
 * ARM EABI toolchain needs divide by zero handler to be implemented
 * externally.
 */
    .globl    __div0
__div0:
    mov    pc, lr
    .globl  raise
raise:
    mov    pc, lr

