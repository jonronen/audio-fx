#include "platform/imx233/system.h"
#include "platform/imx233/system-target.h"
#include "platform/imx233/rtc.h"
#include "platform/imx233/dma.h"
#include "platform/imx233/icoll.h"
#include "platform/imx233/lradc.h"

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

void system_init()
{
    HW_RTC_PERSISTENT4 = 9; /* ??? */

	/* Turn off auto-slow and other tricks */
    __REG_CLR(HW_CLKCTRL_HBUS) = 0x07f00000U;
    HW_CLKCTRL_HBUS = 0x00000002; /* clock divider for HCLK */

    /* power up the clocks */
    imx233_reset_block(&HW_RTC_CTRL);
    __REG_SET(HW_RTC_PERSISTENT0) = 
        HW_RTC_PERSISTENT0__XTAL32KHZ_PWRUP |
        HW_RTC_PERSISTENT0__XTAL24MHZ_PWRUP |
        HW_RTC_PERSISTENT0__CLOCKSOURCE;

    imx233_icoll_init();
}

