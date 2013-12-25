#include "platform/imx233/icoll.h"
#include "platform/imx233/rtc.h"
#include "platform/imx233/serial.h"

isr_t isr_table[INT_SRC_NR_SOURCES*2] CACHEALIGN_ATTR IBSS_ATTR;


/*
 * IRQ handling
 */
void imx233_icoll_ack_irq(unsigned int irq)
{
    volatile int dummy;

    /* Tell ICOLL to release IRQ line */
    HW_ICOLL_VECTOR = 0x0;

    /* ACK current interrupt */
    HW_ICOLL_LEVELACK = HW_ICOLL_LEVELACK__LEVEL0;

    /* Barrier */
    dummy = HW_ICOLL_STAT;
}

void imx233_icoll_set_handler(int src, isr_t handler)
{
    //
    // for some reason, the interrupt handler pitch is 8 bytes
    // instead of 4 bytes (as it should be on startup
    // according to the imx233 manual)
    //

    if ((src >= 0) && (src < INT_SRC_NR_SOURCES))
        isr_table[src*2] = handler;
}

void imx233_icoll_enable_interrupt(int src, bool enable)
{
    if(enable)
        __REG_SET(HW_ICOLL_INTERRUPT(src)) = HW_ICOLL_INTERRUPT__ENABLE;
    else
        __REG_CLR(HW_ICOLL_INTERRUPT(src)) = HW_ICOLL_INTERRUPT__ENABLE;
}

void imx233_icoll_init(void)
{
    int i;

    imx233_reset_block(&HW_ICOLL_CTRL);
    /* disable all interrupts */
    for(i = 0; i < INT_SRC_NR_SOURCES; i++)
    {
        /* priority = 0, disable, disable fiq */
        HW_ICOLL_INTERRUPT(i) = 0;
    }

    // clear all the interrupt handlers
    for (i=0; i<INT_SRC_NR_SOURCES; i++) {
        isr_table[i*2] = 0;
        isr_table[i*2+1] = 0;
    }

    /* setup vbase as isr_table */
    HW_ICOLL_VBASE = (uint32_t)&isr_table;
    /* enable final irq bit */
    __REG_SET(HW_ICOLL_CTRL) = HW_ICOLL_CTRL__IRQ_FINAL_ENABLE;
}

