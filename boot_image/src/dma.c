#include "dma.h"
#include "fx-dma.h"
#include "regs-apbx.h"
#include "regs-apbh.h"
#include "utils.h"

/*
#define HW_APBX_CTRL0            (*((volatile int *)0x80024000))
#define HW_APBX_CTRL2            (*((volatile int *)0x80024020))
#define HW_APBX_CTRL2_SET        (*((volatile int *)0x80024024))
#define HW_APBX_CTRL2_CLR        (*((volatile int *)0x80024028))
#define HW_APBX_CHANNEL_CTRL     (*((volatile int *)0x80024030))
#define HW_APBX_CHANNEL_CTRL_SET (*((volatile int *)0x80024034))
#define HW_APBX_CH3_NXTCMDAR     (*((volatile int *)0x80024260))
#define HW_APBX_CH3_SEMA         (*((volatile int *)0x80024290))
*/

#define AUDIOIN_DMA_CHANNEL  0
#define AUDIOOUT_DMA_CHANNEL 1

struct mx233_dma {
    struct mx233_dma *next_cmd_addr;
    // Configuration format:
    // ################PPPPrrrrWDrrINCC
    // # = Number of bytes to transfer
    // P = Number of PIO words to write
    // r = reserved
    // W = Wait-4-endcmd
    // D = Decrement semaphore
    // I = IRQ complete
    // N = chain
    // C = command
    unsigned int      configuration;

    unsigned char    *buffer;

    // These values will get written to the registers after DMA has
    // completed.  A handy way to start transfers.
    // Because it goes in the order documented, the first byte written here
    // will fill in HW_I2C_CTRL0.
    unsigned long     pio_values[16];
} __attribute__((__packed__));


/* CMD is 0-1 */
#define CMD_CHAIN       (1<<2)
#define CMD_IRQCOMPLETE (1<<3)
#define CMD_SEMAPHORE   (1<<6)
#define CMD_WAIT4END    (1<<7)
/* PIO words are 12-15 */

void stmp3xxx_dma_go(int channel,
             struct stmp3xxx_dma_descriptor *head, u32 semaphore)
{
    int ch = channel % 16;

    switch (channel/16) {
    case STMP3XXX_BUS_APBH:
        /* Set next command */
        HW_APBH_CHn_NXTCMDAR_WR(ch, head->handle);
        /* Set counting semaphore (kicks off transfer). Assumes
           peripheral has been set up correctly */
        HW_APBH_CHn_SEMA_WR(ch, semaphore);
        break;

    case STMP3XXX_BUS_APBX:
        /* Set next command */
        HW_APBX_CHn_NXTCMDAR_WR(ch, head->handle);
        /* Set counting semaphore (kicks off transfer). Assumes
           peripheral has been set up correctly */
        HW_APBX_CHn_SEMA_WR(ch, semaphore);
        break;
    }
}

static int reset_apbx_channel(int channel) {
    int reset_mask = (1<<channel)<<16;
    int i;

    // Reset the APBX block.
    HW_APBX_CTRL0_WR(0x00000000);

    // Reset the I2C channel.
    HW_APBX_CHANNEL_CTRL_SET_WR(reset_mask);
    for(i=0; i<1000; i++) {
        if(!(HW_APBX_CHANNEL_CTRL_RD() & reset_mask)) {
            return 0;
        }
        msleep(1);
    }
    return 1;
}

int fx_dma_init(void)
{
    int ret;

    g_dma_rec_ch = STMP3xxx_DMA(0, STMP3XXX_BUS_APBX);
    g_dma_play_ch = STMP3xxx_DMA(1, STMP3XXX_BUS_APBX);
    ret = stmp3xxx_dma_request(
        g_dma_rec_ch,
        NULL,
        "stmp3xxx adc"
    );
    if (ret) {
        STR("Failed to request recording DMA channel\n");
        return ret;
    }
    else {
        STR("Got DMA channel ");
        HEX(g_dma_rec_ch);
        STR(".\n");
    }
    ret = stmp3xxx_dma_request(
        g_dma_play_ch,
        NULL,
        "stmp3xxx dac"
    );
    if (ret) {
        STR("Failed to request playback DMA channel\n");
        stmp3xxx_dma_release(g_dma_rec_ch);
        return ret;
    }
    else {
        STR("Got DMA channel ");
        HEX(g_dma_play_ch);
        STR(".\n");
    }

    ret = request_irq(IRQ_ADC_DMA, dma_irq_rec_func, 0, "PCM DMA", NULL);
    if (ret) {
        STR("Unable to request DMA recording irq\n");
        stmp3xxx_dma_release(g_dma_rec_ch);
        stmp3xxx_dma_release(g_dma_play_ch);
        return ret;
    }
    ret = request_irq(IRQ_DAC_DMA, dma_irq_play_func, 0, "PCM DMA", NULL);
    if (ret) {
        STR("Unable to request DMA playback irq\n");
        free_irq(IRQ_ADC_DMA, NULL);
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
            STR("Unable to allocate DMA rec. command ");
            HEX(desc);
            STR("\n");
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
            STR("Unable to allocate DMA play command ");
            HEX(desc);
            STR("\n");
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
}

