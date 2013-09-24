#include "system.h"
#include "system-target.h"
#include "lradc-imx233.h"

/* irq callbacks */
static lradc_irq_fn_t irq_cb[HW_LRADC_NUM_CHANNELS];

#define define_cb(x) \
    void INT_LRADC_CH##x(void) \
    { \
        INT_LRADC_CH(x); \
    }

void INT_LRADC_CH(int chan)
{
    if(irq_cb[chan])
        irq_cb[chan](chan);
}

define_cb(0)
define_cb(1)
define_cb(2)
define_cb(3)
define_cb(4)
define_cb(5)
define_cb(6)
define_cb(7)

void imx233_lradc_set_channel_irq_callback(int channel, lradc_irq_fn_t cb)
{
    irq_cb[channel] = cb;
}

void imx233_lradc_setup_channel(int channel, bool div2, bool acc, int nr_samples, int src)
{
    // clear all first
    __REG_CLR(HW_LRADC_CHx(channel)) = 0xFFFFFFFF;

    __REG_CLR(HW_LRADC_CHx(channel)) = HW_LRADC_CHx__NUM_SAMPLES_BM | HW_LRADC_CHx__ACCUMULATE;
    __REG_SET(HW_LRADC_CHx(channel)) = nr_samples << HW_LRADC_CHx__NUM_SAMPLES_BP |
        acc << HW_LRADC_CHx__ACCUMULATE;
    if(div2)
        __REG_SET(HW_LRADC_CTRL2) = HW_LRADC_CTRL2__DIVIDE_BY_TWO(channel);
    else
        __REG_CLR(HW_LRADC_CTRL2) = HW_LRADC_CTRL2__DIVIDE_BY_TWO(channel);
    __REG_CLR(HW_LRADC_CTRL4) = HW_LRADC_CTRL4__LRADCxSELECT_BM(channel);
    __REG_SET(HW_LRADC_CTRL4) = src << HW_LRADC_CTRL4__LRADCxSELECT_BP(channel);
}

void imx233_lradc_setup_delay(int dchan, int trigger_lradc, int trigger_delays,
    int loop_count, int delay)
{
    HW_LRADC_DELAYx(dchan) =
        trigger_lradc << HW_LRADC_DELAYx__TRIGGER_LRADCS_BP |
        trigger_delays << HW_LRADC_DELAYx__TRIGGER_DELAYS_BP |
        loop_count << HW_LRADC_DELAYx__LOOP_COUNT_BP |
        delay << HW_LRADC_DELAYx__DELAY_BP;
}

void imx233_lradc_clear_channel_irq(int channel)
{
    __REG_CLR(HW_LRADC_CTRL1) = HW_LRADC_CTRL1__LRADCx_IRQ(channel);
}

bool imx233_lradc_read_channel_irq(int channel)
{
    return HW_LRADC_CTRL1 & HW_LRADC_CTRL1__LRADCx_IRQ(channel);
}

void imx233_lradc_enable_channel_irq(int channel, bool enable)
{
    if(enable)
        __REG_SET(HW_LRADC_CTRL1) = HW_LRADC_CTRL1__LRADCx_IRQ_EN(channel);
    else
        __REG_CLR(HW_LRADC_CTRL1) = HW_LRADC_CTRL1__LRADCx_IRQ_EN(channel);
    imx233_lradc_clear_channel_irq(channel);
}

void imx233_lradc_kick_channel(int channel)
{
    imx233_lradc_clear_channel_irq(channel);
    __REG_SET(HW_LRADC_CTRL0) = HW_LRADC_CTRL0__SCHEDULE(channel);
}

void imx233_lradc_kick_delay(int dchan)
{
    __REG_SET(HW_LRADC_DELAYx(dchan)) = HW_LRADC_DELAYx__KICK;
}

void imx233_lradc_wait_channel(int channel)
{
    /* wait for completion */
    while(!imx233_lradc_read_channel_irq(channel));
}

int imx233_lradc_read_channel(int channel)
{
    return __XTRACT_EX(HW_LRADC_CHx(channel), HW_LRADC_CHx__VALUE);
}

void imx233_lradc_clear_channel(int channel)
{
    __REG_CLR(HW_LRADC_CHx(channel)) = HW_LRADC_CHx__VALUE_BM;
}

void imx233_lradc_init(void)
{
    // enable block
    imx233_reset_block(&HW_LRADC_CTRL0);
    // disable ground ref
    __REG_CLR(HW_LRADC_CTRL0) = HW_LRADC_CTRL0__ONCHIP_GROUNDREF;
    // disable temperature sensors
    __REG_CLR(HW_LRADC_CTRL2) = HW_LRADC_CTRL2__TEMP_SENSOR_IENABLE0 |
        HW_LRADC_CTRL2__TEMP_SENSOR_IENABLE1;
    __REG_SET(HW_LRADC_CTRL2) = HW_LRADC_CTRL2__TEMPSENSE_PWD;
    // set frequency
    __REG_CLR(HW_LRADC_CTRL3) = HW_LRADC_CTRL3__CYCLE_TIME_BM;
    __REG_SET(HW_LRADC_CTRL3) = HW_LRADC_CTRL3__CYCLE_TIME__6MHz;
}
