#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <mach/regs-audioin.h>
#include <mach/dma.h>
#include <mach/regs-apbx.h>
#include <asm/dma.h>

#define PERIOD_SIZE 128
#define NUM_PERIODS 2


static unsigned char* g_p_snd_buff = NULL;
static int g_dma_ch = -1;
static struct stmp3xxx_dma_descriptor *g_p_dma_cmds = NULL;


static irqreturn_t dma_irq_func(int irq, void* p_dev)
{
    u32 irq_mask = 1;
    if (HW_APBX_CTRL1_RD() & irq_mask) {
        stmp3xxx_dma_clear_interrupt(g_dma_ch);
        printk("pcm period elapsed\n");
    } else
        printk(KERN_WARNING "Unknown interrupt\n");

    return IRQ_HANDLED;
}

static void print_array(void* p_arr, size_t sz, const char* p_str)
{
    char* p_buff = (char*)p_arr;
    int n=0;

    printk(p_str);
    printk(KERN_INFO ":");

    while (n<sz) {
        if (n%16 == 0) printk(KERN_INFO "\n\t%04x ", n);
        if (n%8 == 0) printk(KERN_INFO " ");
        printk("%02x", p_buff[n]);
        ++n;
    }
}

static int __init test_init(void)
{
    int ret = 0;
    int desc;
    int i;
    dma_addr_t dma_addr_phys;

    g_p_snd_buff = kmalloc(PERIOD_SIZE*NUM_PERIODS*2, GFP_KERNEL|GFP_DMA);
    if (g_p_snd_buff == NULL) {
        printk("Error allocating memory\n");
        return -ENOMEM;
    }

    /* Set the audio recorder to use LRADC1, and set to an 8K resistor. */
    HW_AUDIOIN_MICLINE_SET(0x01300000);

    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_FIFO_OVERFLOW_IRQ);
    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_FIFO_UNDERFLOW_IRQ);
    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_FIFO_ERROR_IRQ_EN);
    
    g_dma_ch = STMP3xxx_DMA(0, STMP3XXX_BUS_APBX);
    ret = stmp3xxx_dma_request(
        g_dma_ch,
        NULL /*stmp3xxx_pcm_dev - see if we don't need this */,
        "stmp3xxx adc"
    );
    if (ret) {
        printk(KERN_ERR "Failed to request DMA channel\n");
        return ret;
    }
    else 
        printk("Got DMA channel %d\n", g_dma_ch);

    g_p_dma_cmds = kzalloc(
        sizeof(struct stmp3xxx_dma_descriptor) * NUM_PERIODS,
        GFP_KERNEL
    );
    if (g_p_dma_cmds == NULL) {
        printk(KERN_ERR "Unable to allocate memory\n");
        stmp3xxx_dma_release(g_dma_ch);
        return -ENOMEM;
    }

    ret = request_irq(IRQ_ADC_DMA, dma_irq_func, 0, "STMP3xxx PCM DMA", NULL);
    if (ret) {
        printk(KERN_ERR "Unable to request DMA irq\n");
        stmp3xxx_dma_release(g_dma_ch);
        kfree(g_p_dma_cmds);
        return ret;
    }

    for (desc = 0; desc < NUM_PERIODS; desc++) {
        ret = stmp3xxx_dma_allocate_command(g_dma_ch,
                        &g_p_dma_cmds[desc]);
        if (ret) {
            printk(KERN_ERR "Unable to allocate DMA command %d\n", desc);
            while (--desc >= 0)
                stmp3xxx_dma_free_command(g_dma_ch,
                              &g_p_dma_cmds[desc]);
            kfree(g_p_dma_cmds);
            stmp3xxx_dma_release(g_dma_ch);

            return ret;
        }
    }
    
    print_array(
        g_p_dma_cmds,
        sizeof(struct stmp3xxx_dma_descriptor)*NUM_PERIODS,
        "dma_cmds"
    );
    printk(
        KERN_INFO "g_p_dma_cmds: 0x%08x, phys: 0x%08x\n",
        (unsigned int)g_p_dma_cmds,
        (unsigned int)virt_to_phys(g_p_dma_cmds)
    );

    /* clear completion interrupt */
    stmp3xxx_dma_clear_interrupt(g_dma_ch);
    stmp3xxx_dma_enable_interrupt(g_dma_ch);

    /* Reset DMA channel */
    stmp3xxx_dma_reset_channel(g_dma_ch);

    /* Set up a DMA chain to sent DMA buffer */
    dma_addr_phys = virt_to_phys(g_p_snd_buff);
    printk(
        KERN_INFO "g_p_snd_buff: 0x%08x, dma_phys: 0x%08x\n",
        (unsigned int)g_p_snd_buff, dma_addr_phys
    );
    for (i = 0; i < NUM_PERIODS; i++) {
        int next = (i + 1) % NUM_PERIODS;
        u32 cmd = 0;

        /* Link with previous command */
        g_p_dma_cmds[i].command->next =
                g_p_dma_cmds[next].handle;

        g_p_dma_cmds[i].next_descr =
                &g_p_dma_cmds[next];

        cmd = BF_APBX_CHn_CMD_XFER_COUNT(PERIOD_SIZE*2) |
              BM_APBX_CHn_CMD_IRQONCMPLT |
              BM_APBX_CHn_CMD_CHAIN;
        cmd |= BF_APBX_CHn_CMD_COMMAND(
            BV_APBX_CHn_CMD_COMMAND__DMA_WRITE);

        g_p_dma_cmds[i].command->cmd = cmd;
        g_p_dma_cmds[i].command->buf_ptr = dma_addr_phys;

        /* Next data chunk */
        dma_addr_phys += PERIOD_SIZE*2;
    }

    HW_AUDIOIN_CTRL_SET(0x001f0000);
    HW_AUDIOIN_ADCVOL_WR(0x00000808);
    
    printk(KERN_INFO "HW_AUDIOIN_ADCVOL is 0x%08x\n", HW_AUDIOIN_ADCVOL_RD());
    HW_AUDIOIN_CTRL_SET(BM_AUDIOIN_CTRL_RUN);
    
    stmp3xxx_dma_go(g_dma_ch, g_p_dma_cmds, 1);

    return ret;
}

static void __exit test_exit(void)
{
    if (g_p_snd_buff) {
        kfree(g_p_snd_buff);
        g_p_snd_buff = NULL;
    }

    HW_AUDIOIN_CTRL_CLR(BM_AUDIOIN_CTRL_RUN);
    
    free_irq(IRQ_ADC_DMA, NULL);
    
    /* release DMA channel */
    stmp3xxx_dma_release(g_dma_ch);
}

module_init(test_init);
module_exit(test_exit);

MODULE_AUTHOR("Jon Ronen-Drori (jon_ronen@yahoo.com)");
MODULE_DESCRIPTION("Sound tests for Chumby Hacker Board");
MODULE_LICENSE("GPL");

