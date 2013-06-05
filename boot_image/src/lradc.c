#include "stmp3xxx.h"

static void imx233_reset_lradc(void)
{
    /* soft-reset */
    HW_LRADC_CTRL0_SET(BM_LRADC_CTRL0_SFTRST);
    /* make sure block is gated off */
    while(!(HW_LRADC_CTRL0_RD() & BM_LRADC_CTRL0_CLKGATE));
    /* bring block out of reset */
    HW_LRADC_CTRL0_CLR(BM_LRADC_CTRL0_SFTRST);
    while(HW_LRADC_CTRL0_RD() & BM_LRADC_CTRL0_SFTRST);
    /* make sure clock is running */
    HW_LRADC_CTRL0_CLR(BM_LRADC_CTRL0_CLKGATE);
    while(HW_LRADC_CTRL0_RD() & BM_LRADC_CTRL0_CLKGATE);
}

void init_adc(void)
{
    imx233_reset_lradc();

    // use LRADC channel 4 and DELAY0
    HW_LRADC_CTRL2_SET(BF_LRADC_CTRL2_DIVIDE_BY_TWO(0x10));
    HW_LRADC_DELAYn_SET(0, BF_LRADC_DELAYn_TRIGGER_LRADCS(0x10));
    HW_LRADC_DELAYn_SET(0, BF_LRADC_DELAYn_TRIGGER_DELAYS(0x01));
    HW_LRADC_DELAYn_SET(0, BF_LRADC_DELAYn_DELAY(20)); /* sample at 100Hz */
    HW_LRADC_DELAYn_SET(0, BM_LRADC_DELAYn_KICK); /* start the mothafucka */
}


