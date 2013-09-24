#include "system.h"
#include "system-target.h"
#include "dma-imx233.h"
#include "icoll-imx233.h"
#include "lradc-imx233.h"

bool imx233_us_elapsed(uint32_t ref, unsigned us_delay)
{
    uint32_t cur = HW_DIGCTL_MICROSECONDS;
    if(ref + us_delay <= ref)
        return !(cur > ref) && !(cur < (ref + us_delay));
    else
        return (cur < ref) || cur >= (ref + us_delay);
}

void imx233_reset_block(volatile uint32_t *block_reg)
{
    /* soft-reset */
    __REG_SET(*block_reg) = __BLOCK_SFTRST;
    /* make sure block is gated off */
    while(!(*block_reg & __BLOCK_CLKGATE));
    /* bring block out of reset */
    __REG_CLR(*block_reg) = __BLOCK_SFTRST;
    while(*block_reg & __BLOCK_SFTRST);
    /* make sure clock is running */
    __REG_CLR(*block_reg) = __BLOCK_CLKGATE;
    while(*block_reg & __BLOCK_CLKGATE);
}

void udelay(unsigned us)
{
    uint32_t ref = HW_DIGCTL_MICROSECONDS;
    while(!imx233_us_elapsed(ref, us));
}

