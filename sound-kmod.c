#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <mach/regs-audioin.h>
#include <mach/regs-audioout.h>
#include <mach/dma.h>
#include <mach/regs-apbx.h>
#include <mach/regs-rtc.h>
#include <asm/dma.h>

#define PERIOD_SIZE 4096
#define NUM_PERIODS 3


static unsigned short* g_snd_rec_buff = NULL;
static unsigned short* g_snd_play_buff = NULL;
static int g_dma_rec_ch = -1;
static int g_dma_play_ch = -1;
static struct stmp3xxx_dma_descriptor *g_dma_rec_cmds = NULL;
static struct stmp3xxx_dma_descriptor *g_dma_play_cmds = NULL;

static unsigned char g_rec_index;

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
    //short max, curr;
    unsigned short* rec_buff = 
        &g_snd_rec_buff[g_rec_index*PERIOD_SIZE*2];
    unsigned short* play_buff =
        &g_snd_play_buff[g_rec_index*PERIOD_SIZE*2];

    if (HW_APBX_CTRL1_RD() & irq_mask) {
        stmp3xxx_dma_clear_interrupt(g_dma_rec_ch);

        //max = 0x8000;
        
        //for (i=0; i<PERIOD_SIZE; ++i) {
        //    curr = rec_buff[i*2];
        //    if (curr > max) max=curr;
        //}
        //if (g_rec_index == 0) {
            //printk("recording period elapsed. max=%d\n", max);
        //}
        
        // copy the buffer
        for (i=0; i<PERIOD_SIZE*2; ++i) {
            play_buff[i] = rec_buff[i];
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
    dma_addr_t dma_addr_phys;

    g_snd_rec_buff = kzalloc(PERIOD_SIZE*NUM_PERIODS*4, GFP_KERNEL|GFP_DMA);
    if (g_snd_rec_buff == NULL) {
        printk("Error allocating memory\n");
        return -ENOMEM;
    }
    g_snd_play_buff = kzalloc(PERIOD_SIZE*NUM_PERIODS*4, GFP_KERNEL|GFP_DMA);
    if (g_snd_play_buff == NULL) {
        printk("Error allocating memory\n");
        kfree(g_snd_rec_buff);
        return -ENOMEM;
    }

    imx233_reset_audioout();
    imx233_reset_audioin();

    /* Set the audio recorder to use LRADC1, and set to an 8K resistor. */
    HW_AUDIOIN_MICLINE_SET(0x00000003);

    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_FIFO_OVERFLOW_IRQ);
    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_FIFO_UNDERFLOW_IRQ);
    HW_AUDIOIN_CTRL_SET(BM_AUDIOIN_CTRL_FIFO_ERROR_IRQ_EN);
    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_OFFSET_ENABLE);
    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_HPF_ENABLE);
    
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
        kfree(g_snd_rec_buff);
        kfree(g_snd_play_buff);
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
        kfree(g_snd_rec_buff);
        kfree(g_snd_play_buff);
        return ret;
    }
    else 
        printk("Got DMA channel %d\n", g_dma_play_ch);

    g_dma_rec_cmds = kzalloc(
        sizeof(struct stmp3xxx_dma_descriptor) * NUM_PERIODS,
        GFP_KERNEL
    );
    g_dma_play_cmds = kzalloc(
        sizeof(struct stmp3xxx_dma_descriptor) * NUM_PERIODS,
        GFP_KERNEL
    );
    if ((g_dma_rec_cmds == NULL) || (g_dma_play_cmds == NULL)) {
        printk(KERN_ERR "Unable to allocate memory\n");
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        kfree(g_snd_rec_buff);
        kfree(g_snd_play_buff);
        return -ENOMEM;
    }

    ret = request_irq(IRQ_ADC_DMA, dma_irq_rec_func, 0, "PCM DMA", NULL);
    if (ret) {
        printk(KERN_ERR "Unable to request DMA recording irq\n");
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        kfree(g_dma_rec_cmds);
        kfree(g_dma_play_cmds);
        kfree(g_snd_rec_buff);
        kfree(g_snd_play_buff);
        return ret;
    }
    ret = request_irq(IRQ_DAC_DMA, dma_irq_play_func, 0, "PCM DMA", NULL);
    if (ret) {
        printk(KERN_ERR "Unable to request DMA playback irq\n");
        free_irq(IRQ_ADC_DMA, NULL);
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        kfree(g_dma_rec_cmds);
        kfree(g_dma_play_cmds);
        kfree(g_snd_rec_buff);
        kfree(g_snd_play_buff);
        return ret;
    }
    ret = request_irq(IRQ_DAC_ERROR, audioout_err_func, 0, "DAC Error", NULL);
    if (ret) {
        printk(KERN_ERR "Unable to request DAC error irq\n");
        free_irq(IRQ_DAC_DMA, NULL);
        free_irq(IRQ_ADC_DMA, NULL);
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        kfree(g_dma_rec_cmds);
        kfree(g_dma_play_cmds);
        kfree(g_snd_rec_buff);
        kfree(g_snd_play_buff);
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
        kfree(g_dma_rec_cmds);
        kfree(g_dma_play_cmds);
        kfree(g_snd_rec_buff);
        kfree(g_snd_play_buff);
        return ret;
    }

    g_rec_index = 0;
    for (desc = 0; desc < NUM_PERIODS; desc++) {
        ret = stmp3xxx_dma_allocate_command(g_dma_rec_ch,
                        &g_dma_rec_cmds[desc]);
        if (ret) {
            printk(KERN_ERR "Unable to allocate DMA rec. command %d\n", desc);
            while (--desc >= 0)
                stmp3xxx_dma_free_command(g_dma_rec_ch,
                              &g_dma_rec_cmds[desc]);
            kfree(g_dma_rec_cmds);
            kfree(g_dma_play_cmds);
            stmp3xxx_dma_release(g_dma_rec_ch);
            stmp3xxx_dma_release(g_dma_play_ch);
            kfree(g_snd_rec_buff);
            kfree(g_snd_play_buff);

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
            kfree(g_dma_rec_cmds);
            kfree(g_dma_play_cmds);
            kfree(g_snd_rec_buff);
            kfree(g_snd_play_buff);
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
    dma_addr_phys = virt_to_phys(g_snd_rec_buff);
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
        g_dma_rec_cmds[i].command->buf_ptr = dma_addr_phys;

        /* Next data chunk */
        dma_addr_phys += PERIOD_SIZE*4;
    }
    
    dma_addr_phys = virt_to_phys(g_snd_play_buff);
    for (i = 0; i < NUM_PERIODS; i++) {
        int next = (i+1) % NUM_PERIODS;
        size_t addr_offset = ((i+1)%NUM_PERIODS) * PERIOD_SIZE*4;
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
        g_dma_play_cmds[i].command->buf_ptr = dma_addr_phys+addr_offset;
    }

    //HW_AUDIOIN_CTRL_SET(0x001f0000);
    //HW_AUDIOOUT_CTRL_SET(0x001f0000);
    HW_AUDIOIN_ADCVOL_WR(0x00000c0c);
    HW_AUDIOOUT_DACVOLUME_WR(0x00db00db);

    /* Set word-length to 16-bit */
    HW_AUDIOOUT_CTRL_SET(BM_AUDIOOUT_CTRL_WORD_LENGTH);
    HW_AUDIOIN_CTRL_SET(BM_AUDIOIN_CTRL_WORD_LENGTH);
    /* Set frequencies to 44.1KHz */
    HW_AUDIOIN_ADCSRR_WR(0x10110037);
    HW_AUDIOOUT_DACSRR_WR(0x10110037);
    /* Enable DAC and ADC */
    HW_AUDIOOUT_ANACLKCTRL_CLR(BM_AUDIOOUT_ANACLKCTRL_CLKGATE);
    HW_AUDIOIN_ANACLKCTRL_CLR(BM_AUDIOIN_ANACLKCTRL_CLKGATE);
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
    int desc;

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
    
    if (g_dma_rec_cmds) {
        kfree(g_dma_rec_cmds);
        g_dma_rec_cmds = NULL;
    }
    if (g_dma_play_cmds) {
        kfree(g_dma_play_cmds);
        g_dma_play_cmds = NULL;
    }
    if (g_snd_rec_buff) {
        kfree(g_snd_rec_buff);
        g_snd_rec_buff = NULL;
    }
    if (g_snd_play_buff) {
        kfree(g_snd_play_buff);
        g_snd_play_buff = NULL;
    }
}

module_init(sound_kmod_init);
module_exit(sound_kmod_exit);

MODULE_AUTHOR("Jon Ronen-Drori (jon_ronen@yahoo.com)");
MODULE_DESCRIPTION("Sound tests for Chumby Hacker Board");
MODULE_LICENSE("GPL");

