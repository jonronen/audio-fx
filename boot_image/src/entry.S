/*
 * linux_prep entry code.
 *
 * Entered twice:
 *
 * - Before loading Linux kernel image to SDRAM to check
 *   if CPU is being taken out of standby mode and resume
 *   sleeping kernel.
 * 
 * - After loading Linux kernel image to prepare Linux boot
 *   paramteters and jump to the kernel
 *
 * Vladislav Buzov <vbuzov@embeddedalley.com>
 *
 * Copyright 2008 SigmaTel, Inc
 * Copyright 2008 Embedded Alley Solutions, Inc
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program  is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */
#include "imx233.h"

.section .vectors,"ax",%progbits
.code 32
    /* most handlers are in DRAM which is too far away for a relative jump */
    ldr     pc, =start

.text
.global start
start:

// Save our load address as the global base_address.
    mov r0, pc
    sub r0, r0, #8
    sub r0, r0, #0x2000
    ldr r1, =base_address
    str r0, [r1]


    bl    copy_iram
    bl    clear_ibss

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
    
    /* Enter the Chumby boot shell */
    bl fx_main

    .pool
.global base_address
base_address:
    .long 0xc111b111


    .pool

copy_iram:
    /* Copy the IRAM */
    /* must be done before bss is zeroed */
    ldr     r2, =_iramcopy
    ldr     r3, =_iramstart
    ldr     r4, =_iramend
1:
    cmp     r4, r3
    ldrhi   r5, [r2], #4
    strhi   r5, [r3], #4
    bhi     1b


/*
 * This function clears out the .bss section.
 */
clear_ibss:
    ldr    r0, =_ibss_start
    ldr    r1, =_ibss_end
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
