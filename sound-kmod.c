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

#define PERIOD_SIZE 256
#define NUM_PERIODS 4


static unsigned char* g_p_snd_buff = NULL;
static int g_dma_rec_ch = -1;
static int g_dma_play_ch = -1;
static struct stmp3xxx_dma_descriptor *g_p_dma_rec_cmds = NULL;
static struct stmp3xxx_dma_descriptor *g_p_dma_play_cmds = NULL;


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
    short max, curr;

    if (HW_APBX_CTRL1_RD() & irq_mask) {
        max = 0x8000;
        for (i=0; i<PERIOD_SIZE; ++i) {
            curr = (unsigned short)
                (g_p_snd_buff[i*2] + (g_p_snd_buff[i*2+1]<<8));
            if (curr > max) max=curr;
        }
        //printk("recording period elapsed. max=%d\n", max);
    } else
        printk(KERN_WARNING "Unknown interrupt\n");

    stmp3xxx_dma_clear_interrupt(g_dma_rec_ch);
    return IRQ_HANDLED;
}

static irqreturn_t dma_irq_play_func(int irq, void* p_dev)
{
    u32 irq_mask = 2;

    if (HW_APBX_CTRL1_RD() & irq_mask) {
        //printk(KERN_INFO "playback period elapsed.");
    } else
        printk(KERN_WARNING "Unknown interrupt\n");

    stmp3xxx_dma_clear_interrupt(g_dma_play_ch);
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

    g_p_snd_buff = kzalloc(PERIOD_SIZE*NUM_PERIODS*2, GFP_KERNEL|GFP_DMA);
    if (g_p_snd_buff == NULL) {
        printk("Error allocating memory\n");
        return -ENOMEM;
    }

    //for (i=0; i<NUM_PERIODS; i++) {
    //    memset(g_p_snd_buff + (i*4)*(PERIOD_SIZE/2), 0x00, PERIOD_SIZE/2);
    //    memset(g_p_snd_buff + (i*4+1)*(PERIOD_SIZE/2), 0x40, PERIOD_SIZE/2);
    //    memset(g_p_snd_buff + (i*4+2)*(PERIOD_SIZE/2), 0x80, PERIOD_SIZE/2);
    //    memset(g_p_snd_buff + (i*4+3)*(PERIOD_SIZE/2), 0xC0, PERIOD_SIZE/2);
    //}

    imx233_reset_audioout();
    imx233_reset_audioin();

    /* Set the audio recorder to use LRADC1, and set to an 8K resistor. */
    HW_AUDIOIN_MICLINE_SET(0x01300001);

    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_FIFO_OVERFLOW_IRQ);
    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_FIFO_UNDERFLOW_IRQ);
    HW_AUDIOIN_CTRL_SET(BM_AUDIOIN_CTRL_FIFO_ERROR_IRQ_EN);
    
    HW_AUDIOOUT_CTRL_CLR(BM_AUDIOOUT_CTRL_FIFO_OVERFLOW_IRQ);
    HW_AUDIOOUT_CTRL_CLR(BM_AUDIOOUT_CTRL_FIFO_UNDERFLOW_IRQ);
    HW_AUDIOOUT_CTRL_SET(BM_AUDIOOUT_CTRL_FIFO_ERROR_IRQ_EN);
    
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

    g_p_dma_rec_cmds = kzalloc(
        sizeof(struct stmp3xxx_dma_descriptor) * NUM_PERIODS,
        GFP_KERNEL
    );
    g_p_dma_play_cmds = kzalloc(
        sizeof(struct stmp3xxx_dma_descriptor) * NUM_PERIODS,
        GFP_KERNEL
    );
    if ((g_p_dma_rec_cmds == NULL) || (g_p_dma_play_cmds == NULL)) {
        printk(KERN_ERR "Unable to allocate memory\n");
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        return -ENOMEM;
    }

    ret = request_irq(IRQ_ADC_DMA, dma_irq_rec_func, 0, "PCM DMA", NULL);
    if (ret) {
        printk(KERN_ERR "Unable to request DMA recording irq\n");
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        kfree(g_p_dma_rec_cmds);
        kfree(g_p_dma_play_cmds);
        return ret;
    }
    ret = request_irq(IRQ_DAC_DMA, dma_irq_play_func, 0, "PCM DMA", NULL);
    if (ret) {
        printk(KERN_ERR "Unable to request DMA playback irq\n");
        free_irq(IRQ_ADC_DMA, NULL);
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        kfree(g_p_dma_rec_cmds);
        kfree(g_p_dma_play_cmds);
        return ret;
    }
    ret = request_irq(IRQ_DAC_ERROR, audioout_err_func, 0, "DAC Error", NULL);
    if (ret) {
        printk(KERN_ERR "Unable to request DAC error irq\n");
        free_irq(IRQ_DAC_DMA, NULL);
        free_irq(IRQ_ADC_DMA, NULL);
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        kfree(g_p_dma_rec_cmds);
        kfree(g_p_dma_play_cmds);
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
        kfree(g_p_dma_rec_cmds);
        kfree(g_p_dma_play_cmds);
        return ret;
    }

    for (desc = 0; desc < NUM_PERIODS; desc++) {
        ret = stmp3xxx_dma_allocate_command(g_dma_rec_ch,
                        &g_p_dma_rec_cmds[desc]);
        if (ret) {
            printk(KERN_ERR "Unable to allocate DMA rec. command %d\n", desc);
            while (--desc >= 0)
                stmp3xxx_dma_free_command(g_dma_rec_ch,
                              &g_p_dma_rec_cmds[desc]);
            kfree(g_p_dma_rec_cmds);
            kfree(g_p_dma_play_cmds);
            stmp3xxx_dma_release(g_dma_rec_ch);
            stmp3xxx_dma_release(g_dma_play_ch);

            return ret;
        }
    }
    for (desc = 0; desc < NUM_PERIODS; desc++) {
        ret = stmp3xxx_dma_allocate_command(g_dma_play_ch,
                        &g_p_dma_play_cmds[desc]);
        if (ret) {
            printk(KERN_ERR "Unable to allocate DMA play command %d\n", desc);
            while (--desc >= 0)
                stmp3xxx_dma_free_command(g_dma_play_ch,
                              &g_p_dma_play_cmds[desc]);
            desc=NUM_PERIODS;
            while (--desc >= 0)
                stmp3xxx_dma_free_command(g_dma_rec_ch,
                              &g_p_dma_rec_cmds[desc]);
            kfree(g_p_dma_rec_cmds);
            kfree(g_p_dma_play_cmds);
            stmp3xxx_dma_release(g_dma_rec_ch);
            stmp3xxx_dma_release(g_dma_play_ch);

            return ret;
        }
    }
    
    //print_array(
    //    g_p_dma_cmds,
    //    sizeof(struct stmp3xxx_dma_descriptor)*NUM_PERIODS,
    //    "dma_cmds"
    //);
    //printk(
    //    KERN_INFO "g_p_dma_cmds: 0x%08x, phys: 0x%08x\n",
    //    (unsigned int)g_p_dma_cmds,
    //    (unsigned int)virt_to_phys(g_p_dma_cmds)
    //);

    /* clear completion interrupt */
    stmp3xxx_dma_clear_interrupt(g_dma_rec_ch);
    stmp3xxx_dma_enable_interrupt(g_dma_rec_ch);
    stmp3xxx_dma_clear_interrupt(g_dma_play_ch);
    stmp3xxx_dma_enable_interrupt(g_dma_play_ch);

    /* Reset DMA channel */
    stmp3xxx_dma_reset_channel(g_dma_rec_ch);
    stmp3xxx_dma_reset_channel(g_dma_play_ch);

    /* Set up a DMA chain to sent DMA buffer */
    dma_addr_phys = virt_to_phys(g_p_snd_buff);
    //printk(
    //    KERN_INFO "g_p_snd_buff: 0x%08x, dma_phys: 0x%08x\n",
    //    (unsigned int)g_p_snd_buff, dma_addr_phys
    //);
    for (i = 0; i < NUM_PERIODS; i++) {
        int next = (i + 1) % NUM_PERIODS;
        u32 cmd = 0;

        /* Link with previous command */
        g_p_dma_rec_cmds[i].command->next =
                g_p_dma_rec_cmds[next].handle;

        g_p_dma_rec_cmds[i].next_descr =
                &g_p_dma_rec_cmds[next];

        cmd = BF_APBX_CHn_CMD_XFER_COUNT(PERIOD_SIZE*2) |
              BM_APBX_CHn_CMD_CHAIN;
        cmd |= BF_APBX_CHn_CMD_COMMAND(
            BV_APBX_CHn_CMD_COMMAND__DMA_WRITE);
        if (i==0) cmd |= BM_APBX_CHn_CMD_IRQONCMPLT;

        g_p_dma_rec_cmds[i].command->cmd = cmd;
        g_p_dma_rec_cmds[i].command->buf_ptr = dma_addr_phys;

        /* Next data chunk */
        dma_addr_phys += PERIOD_SIZE*2;
    }
    dma_addr_phys -= (PERIOD_SIZE*2*NUM_PERIODS);
    for (i = 0; i < NUM_PERIODS; i++) {
        int next = (i+1) % NUM_PERIODS;
        size_t addr_offset = ((i+2) % NUM_PERIODS) * PERIOD_SIZE*2;
        u32 cmd = 0;

        /* Link with previous command */
        g_p_dma_play_cmds[i].command->next =
                g_p_dma_play_cmds[next].handle;

        g_p_dma_play_cmds[i].next_descr =
                &g_p_dma_play_cmds[next];

        cmd = BF_APBX_CHn_CMD_XFER_COUNT(PERIOD_SIZE*2) |
              BM_APBX_CHn_CMD_CHAIN;
        cmd |= BF_APBX_CHn_CMD_COMMAND(
            BV_APBX_CHn_CMD_COMMAND__DMA_READ);
        if (i==0) cmd |= BM_APBX_CHn_CMD_IRQONCMPLT;

        g_p_dma_play_cmds[i].command->cmd = cmd;
        g_p_dma_play_cmds[i].command->buf_ptr = dma_addr_phys+addr_offset;
    }

    print_array(
        g_p_dma_play_cmds,
        sizeof(struct stmp3xxx_dma_descriptor)*NUM_PERIODS,
        "g_p_dma_play_cmds"
    );
    printk(
        "g_p_dma_play_cmds: 0x%lx, phys: 0x%lx\n",
        (long unsigned int)g_p_dma_play_cmds, virt_to_phys(g_p_dma_play_cmds)
    );

    HW_AUDIOIN_CTRL_SET(0x001f0000);
    HW_AUDIOOUT_CTRL_SET(0x001f0000);
    HW_AUDIOIN_ADCVOL_WR(0x00000c0c);
    HW_AUDIOOUT_DACVOLUME_WR(0x02db00db);

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
    /* Set capless mode */
    HW_AUDIOOUT_PWRDN_CLR(BM_AUDIOOUT_PWRDN_CAPLESS);
    /* Power up DAC, ADC, speaker, and headphones */
    HW_AUDIOOUT_PWRDN_CLR(BM_AUDIOOUT_PWRDN_DAC);
    HW_AUDIOOUT_PWRDN_CLR(BM_AUDIOOUT_PWRDN_ADC);
    HW_AUDIOOUT_PWRDN_CLR(BM_AUDIOOUT_PWRDN_SPEAKER);
    HW_AUDIOOUT_PWRDN_CLR(BM_AUDIOOUT_PWRDN_HEADPHONE);
    /* release HP from ground */
    //HW_RTC_PERSISTENT0_CLR(0x00080000);
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
    HW_AUDIOIN_CTRL_SET(BM_AUDIOIN_CTRL_RUN);
    stmp3xxx_dma_go(g_dma_rec_ch, g_p_dma_rec_cmds, 1);
    stmp3xxx_dma_go(g_dma_play_ch, g_p_dma_play_cmds, 1);
    HW_AUDIOOUT_CTRL_SET(BM_AUDIOOUT_CTRL_RUN);

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
            &g_p_dma_rec_cmds[desc]
        );
        stmp3xxx_dma_free_command(
            g_dma_play_ch,
            &g_p_dma_play_cmds[desc]
        );
    }

    /* release DMA channel */
    stmp3xxx_dma_release(g_dma_rec_ch);
    stmp3xxx_dma_release(g_dma_play_ch);
    
    if (g_p_dma_rec_cmds) {
        kfree(g_p_dma_rec_cmds);
        g_p_dma_rec_cmds = NULL;
    }
    if (g_p_dma_play_cmds) {
        kfree(g_p_dma_play_cmds);
        g_p_dma_play_cmds = NULL;
    }
    if (g_p_snd_buff) {
        kfree(g_p_snd_buff);
        g_p_snd_buff = NULL;
    }
}

module_init(sound_kmod_init);
module_exit(sound_kmod_exit);

MODULE_AUTHOR("Jon Ronen-Drori (jon_ronen@yahoo.com)");
MODULE_DESCRIPTION("Sound tests for Chumby Hacker Board");
MODULE_LICENSE("GPL");

