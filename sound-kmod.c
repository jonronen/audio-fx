#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <mach/regs-audioin.h>
#include <mach/regs-audioout.h>
#include <mach/regs-lradc.h>
#include <mach/lradc.h>
#include <mach/dma.h>
#include <mach/regs-apbx.h>
#include <mach/regs-rtc.h>
#include <asm/dma.h>


#define PERIOD_SIZE 512
#define NUM_PERIODS 3


static int* g_snd_rec_buffs[NUM_PERIODS];
static int* g_snd_play_buffs[NUM_PERIODS];
static dma_addr_t g_snd_rec_phys[NUM_PERIODS];
static dma_addr_t g_snd_play_phys[NUM_PERIODS];
static int g_dma_rec_ch = -1;
static int g_dma_play_ch = -1;
static struct stmp3xxx_dma_descriptor g_dma_rec_cmds[NUM_PERIODS];
static struct stmp3xxx_dma_descriptor g_dma_play_cmds[NUM_PERIODS];

static unsigned char print_cnt;
static unsigned char g_rec_index;
static unsigned int g_first_interrupt;

static void imx233_reset_audioin(void)
{
    /* soft-reset */
    HW_AUDIOIN_CTRL_SET(BM_AUDIOIN_CTRL_SFTRST);
    /* make sure block is gated off */
    while(!(HW_AUDIOIN_CTRL_RD() & BM_AUDIOIN_CTRL_CLKGATE));
    /* bring block out of reset */
    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_SFTRST);
    while(HW_AUDIOIN_CTRL_RD() & BM_AUDIOIN_CTRL_SFTRST);
    /* make sure clock is running */
    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_CLKGATE);
    while(HW_AUDIOIN_CTRL_RD() & BM_AUDIOIN_CTRL_CLKGATE);
}

static void imx233_reset_audioout(void)
{
    /* soft-reset */
    HW_AUDIOOUT_CTRL_SET(BM_AUDIOOUT_CTRL_SFTRST);
    /* make sure block is gated off */
    while(!(HW_AUDIOOUT_CTRL_RD() & BM_AUDIOOUT_CTRL_CLKGATE));
    /* bring block out of reset */
    HW_AUDIOOUT_CTRL_CLR(BM_AUDIOOUT_CTRL_SFTRST);
    while(HW_AUDIOOUT_CTRL_RD() & BM_AUDIOOUT_CTRL_SFTRST);
    /* make sure clock is running */
    HW_AUDIOOUT_CTRL_CLR(BM_AUDIOOUT_CTRL_CLKGATE);
    while(HW_AUDIOOUT_CTRL_RD() & BM_AUDIOOUT_CTRL_CLKGATE);
}

static void imx233_init_adc(void)
{
    hw_lradc_use_channel(LRADC_CH4);
    hw_lradc_init_ladder(LRADC_CH4, 0, 20); /* sample at 100Hz */
    /* don't use IRQs for the ADC. just poll manually */
    HW_LRADC_CTRL1_CLR(BM_LRADC_CTRL1_LRADC4_IRQ);
    HW_LRADC_CTRL1_CLR(BM_LRADC_CTRL1_LRADC4_IRQ_EN);
    hw_lradc_set_delay_trigger_kick(0, !0);
}

static irqreturn_t audioout_err_func(int irq, void* p_dev)
{
    if (HW_AUDIOOUT_CTRL_RD() & BM_AUDIOOUT_CTRL_FIFO_UNDERFLOW_IRQ) {
        printk(KERN_DEBUG "AUDIOOUT underflow detected\n");
        HW_AUDIOOUT_CTRL_CLR(BM_AUDIOOUT_CTRL_FIFO_UNDERFLOW_IRQ);
    }
    else
        printk(KERN_WARNING "Unknown AUDIOOUT error interrupt\n");

    return IRQ_HANDLED;
}

static irqreturn_t audioin_err_func(int irq, void* p_dev)
{
    if (HW_AUDIOIN_CTRL_RD() & BM_AUDIOIN_CTRL_FIFO_OVERFLOW_IRQ) {
        printk(KERN_DEBUG "AUDIOIN overflow detected\n");
        HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_FIFO_OVERFLOW_IRQ);
    }
    else
        printk(KERN_WARNING "Unknown AUDIOIN error interrupt\n");

    return IRQ_HANDLED;
}

static irqreturn_t dma_irq_rec_func(int irq, void* p_dev)
{
    u32 irq_mask = 1;
    int i;
    int min, max, curr;

    if (HW_APBX_CTRL1_RD() & irq_mask) {
        stmp3xxx_dma_clear_interrupt(g_dma_rec_ch);

        if (g_first_interrupt) {
            g_first_interrupt = 0;
            HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_OFFSET_ENABLE);
        }

        max = -0x7fffffff;
        min = 0x7fffffff;
        
        for (i=0; i<PERIOD_SIZE; i+=2) {
            curr = g_snd_rec_buffs[g_rec_index][i];
            if (curr > max) max=curr;
            if (curr < min) min=curr;
        }
        if (g_rec_index == 0) {
            print_cnt++;
            if (print_cnt == 0) {
                printk("recording period elapsed. min=%d, max=%d\n", min, max);
            }
        }
        
        // copy the buffer
        for (i=0; i<PERIOD_SIZE; ++i) {
            g_snd_play_buffs[g_rec_index][i] = g_snd_rec_buffs[g_rec_index][i];
        }
        
        g_rec_index = (g_rec_index+1)%NUM_PERIODS;
    } else
        printk(KERN_WARNING "Unknown interrupt\n");

    return IRQ_HANDLED;
}

static irqreturn_t dma_irq_play_func(int irq, void* p_dev)
{
    u32 irq_mask = 2;

    if (HW_APBX_CTRL1_RD() & irq_mask) {
        stmp3xxx_dma_clear_interrupt(g_dma_play_ch);
        //printk(KERN_INFO "playback period elapsed.\n");
    } else
        printk(KERN_WARNING "Unknown interrupt\n");

    return IRQ_HANDLED;
}

static void print_array(void* p_arr, size_t sz, const char* p_str)
{
    char* p_buff = (char*)p_arr;
    int n=0;

    printk(KERN_INFO);
    printk(p_str);
    printk(":");

    while (n<sz) {
        if (n%16 == 0) printk("\n\t%04x ", n);
        if (n%8 == 0) printk(" ");
        printk("%02x", p_buff[n]);
        ++n;
    }
    printk("\n");
}

static int __init sound_kmod_init(void)
{
    int ret = 0;
    int desc;
    int i;

    g_first_interrupt = 1;
    
    for (i=0; i<NUM_PERIODS; i++) {
        g_snd_rec_buffs[i] = dma_alloc_coherent(NULL, PERIOD_SIZE*4, 
            &g_snd_rec_phys[i], GFP_KERNEL);
        g_snd_play_buffs[i] = dma_alloc_coherent(NULL, PERIOD_SIZE*4, 
            &g_snd_play_phys[i], GFP_KERNEL);
    }
    if ((g_snd_rec_buffs[0] == NULL) || (g_snd_play_buffs[0] == NULL)) {
        printk("Error allocating memory\n");
        return -ENOMEM;
    }

    imx233_reset_audioout();
    imx233_reset_audioin();
    imx233_init_adc();

    /* Set the audio recorder to use LRADC1, and set to an 8K resistor. */
    HW_AUDIOIN_MICLINE_WR(0x00000001);

    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_FIFO_OVERFLOW_IRQ);
    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_FIFO_UNDERFLOW_IRQ);
    HW_AUDIOIN_CTRL_SET(BM_AUDIOIN_CTRL_FIFO_ERROR_IRQ_EN);
    HW_AUDIOIN_CTRL_SET(BM_AUDIOIN_CTRL_OFFSET_ENABLE);
    HW_AUDIOIN_CTRL_SET(BM_AUDIOIN_CTRL_HPF_ENABLE);
    
    HW_AUDIOOUT_CTRL_CLR(BM_AUDIOOUT_CTRL_FIFO_OVERFLOW_IRQ);
    HW_AUDIOOUT_CTRL_CLR(BM_AUDIOOUT_CTRL_FIFO_UNDERFLOW_IRQ);
    HW_AUDIOOUT_CTRL_SET(BM_AUDIOOUT_CTRL_FIFO_ERROR_IRQ_EN);

    // set the frequencies to 44.1KHz
    HW_AUDIOIN_ADCSRR_WR(0x10110037);
    HW_AUDIOOUT_DACSRR_WR(0x10110037);

    g_dma_rec_ch = STMP3xxx_DMA(0, STMP3XXX_BUS_APBX);
    g_dma_play_ch = STMP3xxx_DMA(1, STMP3XXX_BUS_APBX);
    ret = stmp3xxx_dma_request(
        g_dma_rec_ch,
        NULL,
        "stmp3xxx adc"
    );
    if (ret) {
        printk(KERN_ERR "Failed to request recording DMA channel\n");
        return ret;
    }
    else 
        printk("Got DMA channel %d\n", g_dma_rec_ch);
    ret = stmp3xxx_dma_request(
        g_dma_play_ch,
        NULL,
        "stmp3xxx dac"
    );
    if (ret) {
        printk(KERN_ERR "Failed to request playback DMA channel\n");
        stmp3xxx_dma_release(g_dma_rec_ch);
        return ret;
    }
    else 
        printk("Got DMA channel %d\n", g_dma_play_ch);

    ret = request_irq(IRQ_ADC_DMA, dma_irq_rec_func, 0, "PCM DMA", NULL);
    if (ret) {
        printk(KERN_ERR "Unable to request DMA recording irq\n");
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        return ret;
    }
    ret = request_irq(IRQ_DAC_DMA, dma_irq_play_func, 0, "PCM DMA", NULL);
    if (ret) {
        printk(KERN_ERR "Unable to request DMA playback irq\n");
        free_irq(IRQ_ADC_DMA, NULL);
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        return ret;
    }
    ret = request_irq(IRQ_DAC_ERROR, audioout_err_func, 0, "DAC Error", NULL);
    if (ret) {
        printk(KERN_ERR "Unable to request DAC error irq\n");
        free_irq(IRQ_DAC_DMA, NULL);
        free_irq(IRQ_ADC_DMA, NULL);
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        return ret;
    }
    ret = request_irq(IRQ_ADC_ERROR, audioin_err_func, 0, "ADC Error", NULL);
    if (ret) {
        printk(KERN_ERR "Unable to request ADC error irq\n");
        free_irq(IRQ_DAC_DMA, NULL);
        free_irq(IRQ_ADC_DMA, NULL);
        free_irq(IRQ_DAC_ERROR, NULL);
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        return ret;
    }

    print_cnt = 0;
    g_rec_index = 0;
    for (desc = 0; desc < NUM_PERIODS; desc++) {
        ret = stmp3xxx_dma_allocate_command(g_dma_rec_ch,
                        &g_dma_rec_cmds[desc]);
        if (ret) {
            printk(KERN_ERR "Unable to allocate DMA rec. command %d\n", desc);
            while (--desc >= 0)
                stmp3xxx_dma_free_command(g_dma_rec_ch,
                              &g_dma_rec_cmds[desc]);
            stmp3xxx_dma_release(g_dma_rec_ch);
            stmp3xxx_dma_release(g_dma_play_ch);

            return ret;
        }
    }
    for (desc = 0; desc < NUM_PERIODS; desc++) {
        ret = stmp3xxx_dma_allocate_command(g_dma_play_ch,
                        &g_dma_play_cmds[desc]);
        if (ret) {
            printk(KERN_ERR "Unable to allocate DMA play command %d\n", desc);
            while (--desc >= 0)
                stmp3xxx_dma_free_command(g_dma_play_ch,
                              &g_dma_play_cmds[desc]);
            desc=NUM_PERIODS;
            while (--desc >= 0)
                stmp3xxx_dma_free_command(g_dma_rec_ch,
                              &g_dma_rec_cmds[desc]);
            stmp3xxx_dma_release(g_dma_rec_ch);
            stmp3xxx_dma_release(g_dma_play_ch);

            return ret;
        }
    }
    
    /* clear completion interrupt */
    stmp3xxx_dma_clear_interrupt(g_dma_rec_ch);
    stmp3xxx_dma_enable_interrupt(g_dma_rec_ch);
    stmp3xxx_dma_clear_interrupt(g_dma_play_ch);
    stmp3xxx_dma_enable_interrupt(g_dma_play_ch);

    /* Reset DMA channel */
    stmp3xxx_dma_reset_channel(g_dma_rec_ch);
    stmp3xxx_dma_reset_channel(g_dma_play_ch);

    /* Set up a DMA chain to sent DMA buffer */
    for (i = 0; i < NUM_PERIODS; i++) {
        int next = (i+1) % NUM_PERIODS;
        u32 cmd = 0;

        /* Link with previous command */
        g_dma_rec_cmds[i].command->next =
                g_dma_rec_cmds[next].handle;

        g_dma_rec_cmds[i].next_descr =
                &g_dma_rec_cmds[next];

        cmd = BF_APBX_CHn_CMD_XFER_COUNT(PERIOD_SIZE*4) |
              BM_APBX_CHn_CMD_CHAIN;
        cmd |= BF_APBX_CHn_CMD_COMMAND(
            BV_APBX_CHn_CMD_COMMAND__DMA_WRITE);
        cmd |= BM_APBX_CHn_CMD_IRQONCMPLT;

        g_dma_rec_cmds[i].command->cmd = cmd;
        g_dma_rec_cmds[i].command->buf_ptr = g_snd_rec_phys[i];
    }
    
    for (i = 0; i < NUM_PERIODS; i++) {
        int next = (i+1) % NUM_PERIODS;
        u32 cmd = 0;

        /* Link with previous command */
        g_dma_play_cmds[i].command->next =
                g_dma_play_cmds[next].handle;

        g_dma_play_cmds[i].next_descr =
                &g_dma_play_cmds[next];

        cmd = BF_APBX_CHn_CMD_XFER_COUNT(PERIOD_SIZE*4) |
              BM_APBX_CHn_CMD_CHAIN;
        cmd |= BF_APBX_CHn_CMD_COMMAND(
            BV_APBX_CHn_CMD_COMMAND__DMA_READ);
        cmd |= BM_APBX_CHn_CMD_IRQONCMPLT;

        g_dma_play_cmds[i].command->cmd = cmd;
        g_dma_play_cmds[i].command->buf_ptr = g_snd_play_phys[(i+1)%NUM_PERIODS];
    }

    HW_AUDIOIN_CTRL_SET(0x001f0000);
    HW_AUDIOOUT_CTRL_SET(0x001f0000);
    HW_AUDIOIN_ADCVOL_WR(0x00000202);
    HW_AUDIOIN_ADCVOLUME_WR(0x00db00db);
    HW_AUDIOOUT_DACVOLUME_WR(0x00ff00ff);

    /* Set word-length to 32-bit */
    HW_AUDIOOUT_CTRL_CLR(BM_AUDIOOUT_CTRL_WORD_LENGTH);
    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_WORD_LENGTH);
    /* Set frequencies to 44.1KHz */
    HW_AUDIOIN_ADCSRR_WR(0x10110037);
    HW_AUDIOOUT_DACSRR_WR(0x10110037);
    /* Enable DAC and ADC */
    HW_AUDIOOUT_ANACLKCTRL_CLR(BM_AUDIOOUT_ANACLKCTRL_CLKGATE);
    HW_AUDIOIN_ANACLKCTRL_CLR(BM_AUDIOIN_ANACLKCTRL_CLKGATE);
    /* set the diethering. they say it improves quality */
    HW_AUDIOIN_ANACLKCTRL_CLR(BM_AUDIOIN_ANACLKCTRL_DITHER_OFF);
    /* Hold HP to ground to avoid pop, then release and power up stuff */
    HW_AUDIOOUT_ANACTRL_SET(BM_AUDIOOUT_ANACTRL_HP_HOLD_GND);
    HW_RTC_PERSISTENT0_SET(0x00080000);
    ///* Set capless mode */
    //HW_AUDIOOUT_PWRDN_CLR(BM_AUDIOOUT_PWRDN_CAPLESS);
    /* Power up DAC, ADC, speaker, and headphones */
    HW_AUDIOOUT_PWRDN_CLR(BM_AUDIOOUT_PWRDN_DAC);
    HW_AUDIOOUT_PWRDN_CLR(BM_AUDIOOUT_PWRDN_ADC);
    HW_AUDIOOUT_PWRDN_CLR(BM_AUDIOOUT_PWRDN_SPEAKER);
    HW_AUDIOOUT_PWRDN_CLR(BM_AUDIOOUT_PWRDN_HEADPHONE);
    /* release HP from ground */
    HW_RTC_PERSISTENT0_CLR(0x00080000);
    
    /* Set HP mode to AB */
    HW_AUDIOOUT_ANACTRL_SET(BM_AUDIOOUT_ANACTRL_HP_CLASSAB);
    /* Stop holding to ground */
    HW_AUDIOOUT_ANACTRL_CLR(BM_AUDIOOUT_ANACTRL_HP_HOLD_GND);

    HW_AUDIOOUT_HPVOL_CLR(BM_AUDIOOUT_HPVOL_MUTE);
    HW_AUDIOOUT_HPVOL_CLR(0x0000FFFF);
    HW_AUDIOOUT_SPEAKERCTRL_SET(BM_AUDIOOUT_SPEAKERCTRL_MUTE);
    
    // write some shit to the data to make sure it's not stuck
    HW_AUDIOOUT_DATA_WR(0x00000000);
    udelay(200);
    HW_AUDIOOUT_DATA_WR(0x00000001);
    udelay(200);
    HW_AUDIOOUT_CTRL_SET(BM_AUDIOOUT_CTRL_RUN);
    stmp3xxx_dma_go(g_dma_play_ch, g_dma_play_cmds, 1);
    HW_AUDIOIN_CTRL_SET(BM_AUDIOIN_CTRL_RUN);
    stmp3xxx_dma_go(g_dma_rec_ch, g_dma_rec_cmds, 1);

    return ret;
}

static void __exit sound_kmod_exit(void)
{
    int desc, i;

    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_RUN);
    HW_AUDIOOUT_CTRL_CLR(BM_AUDIOIN_CTRL_RUN);
    
    /* TODO: stall the DMA channel (use the semaphore?) */
    
    free_irq(IRQ_ADC_DMA, NULL);
    free_irq(IRQ_DAC_DMA, NULL);
    free_irq(IRQ_ADC_ERROR, NULL);
    free_irq(IRQ_DAC_ERROR, NULL);
    
    for (desc = 0; desc < NUM_PERIODS; desc++) {
        stmp3xxx_dma_free_command(
            g_dma_rec_ch,
            &g_dma_rec_cmds[desc]
        );
        stmp3xxx_dma_free_command(
            g_dma_play_ch,
            &g_dma_play_cmds[desc]
        );
    }

    /* release DMA channel */
    stmp3xxx_dma_release(g_dma_rec_ch);
    stmp3xxx_dma_release(g_dma_play_ch);
    
    for (i=0; i<NUM_PERIODS; i++) {
        if (g_snd_rec_buffs[i]) {
            dma_free_coherent(NULL, PERIOD_SIZE*4,
                g_snd_rec_buffs[i], g_snd_rec_phys[i]);
            g_snd_rec_buffs[i] = NULL;
        }
        if (g_snd_play_buffs[i]) {
            dma_free_coherent(NULL, PERIOD_SIZE*4,
                g_snd_play_buffs[i], g_snd_play_phys[i]);
            g_snd_play_buffs[i] = NULL;
        }
    }

    /* stop the ADC scheduling */
    hw_lradc_set_delay_trigger_kick(0, 0);
}

module_init(sound_kmod_init);
module_exit(sound_kmod_exit);

MODULE_AUTHOR("Jon Ronen-Drori (jon_ronen@yahoo.com)");
MODULE_DESCRIPTION("Sound tests for Chumby Hacker Board");
MODULE_LICENSE("GPL");

