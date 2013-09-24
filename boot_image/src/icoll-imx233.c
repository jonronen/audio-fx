#include "icoll-imx233.h"
#include "rtc-imx233.h"
#include "serial.h"

isr_t isr_table[INT_SRC_NR_SOURCES] CACHEALIGN_ATTR IBSS_ATTR;


isr_t isr_table[INT_SRC_NR_SOURCES] =
{0};


/*
 * IRQ handling
 */
void stmp378x_ack_irq(unsigned int irq)
{
    volatile int dummy;

    /* Tell ICOLL to release IRQ line */
    HW_ICOLL_VECTOR = 0x0;

    /* ACK current interrupt */
    HW_ICOLL_LEVELACK = HW_ICOLL_LEVELACK__LEVEL0;

    /* Barrier */
    dummy = HW_ICOLL_STAT;
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
    /* setup vbase as isr_table */
    HW_ICOLL_VBASE = (uint32_t)&isr_table;
    /* enable final irq bit */
    __REG_SET(HW_ICOLL_CTRL) = HW_ICOLL_CTRL__IRQ_FINAL_ENABLE;
}

