/* 
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program  is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include "serial.h"
#include "dma-imx233.h"
#include "audioin-imx233.h"
#include "audioout-imx233.h"
#include "rtc-imx233.h"
#include "icoll-imx233.h"
#include "lradc-imx233.h"
#include "system-arm.h"


static uint8_t g_print_cnt;
static uint8_t g_rec_index;
static bool g_first_interrupt;


struct audio_dma_command_t
{
    struct apb_dma_command_t dma;
    /* no PIO words, just pad to 32 bytes for the cache line */
    uint32_t pad[5];
} __attribute__((packed)) CACHEALIGN_ATTR;

__ENSURE_STRUCT_CACHE_FRIENDLY(struct audio_dma_command_t)


#define NUM_PERIODS 3
#define NUM_SAMPLES 128


static struct audio_dma_command_t g_audio_play_cmds[NUM_PERIODS];
static struct audio_dma_command_t g_audio_rec_cmds[NUM_PERIODS];


static void dac_dma_interrupt(void);
static void adc_dma_interrupt(void);
static void dac_error_interrupt(void);
static void adc_error_interrupt(void);

/* keep the audio buffers in the internal RAM for faster performance */
static int32_t g_rec_buff[NUM_PERIODS][NUM_SAMPLES*2] \
    CACHEALIGN_ATTR IBSS_ATTR;
static int32_t g_play_buff[NUM_PERIODS][NUM_SAMPLES*2] \
    CACHEALIGN_ATTR IBSS_ATTR;


static void audio_regs_init()
{
    imx233_reset_block(&HW_AUDIOIN_CTRL);
    imx233_reset_block(&HW_AUDIOOUT_CTRL);
    /* remove the gate-off for DIGFILT */
    __REG_CLR(HW_CLKCTRL_XTAL) = 0x40000000;

    /* Set the audio recorder to use LRADC1, and set to an 8K resistor. */
    __REG_SET(HW_AUDIOIN_MICLINE) = 0x00000001;

    __REG_CLR(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__FIFO_OVERFLOW_IRQ;
    __REG_CLR(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__FIFO_UNDERFLOW_IRQ;
    //__REG_SET(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__FIFO_ERROR_IRQ_EN;
    __REG_SET(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__OFFSET_ENABLE;
    __REG_SET(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__HPF_ENABLE;

    __REG_CLR(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__FIFO_OVERFLOW_IRQ;
    __REG_CLR(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__FIFO_UNDERFLOW_IRQ;
    //__REG_SET(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__FIFO_ERROR_IRQ_EN;

    // set the frequencies to 44.1KHz
    HW_AUDIOIN_ADCSRR = 0x10110037;
    HW_AUDIOOUT_DACSRR = 0x10110037;

    __REG_SET(HW_AUDIOIN_CTRL) = 0x001f0000;
    __REG_SET(HW_AUDIOOUT_CTRL) = 0x001f0000;
    HW_AUDIOIN_ADCVOL = 0x00000202;
    HW_AUDIOIN_ADCVOLUME = 0x00db00db;
    HW_AUDIOOUT_DACVOLUME = 0x00ff00ff;

    /* Set word-length to 32-bit */
    __REG_CLR(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__WORD_LENGTH;
    __REG_CLR(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__WORD_LENGTH;
    /* Enable DAC and ADC */
    __REG_CLR(HW_AUDIOOUT_ANACLKCTRL) = HW_AUDIOOUT_ANACLKCTRL__CLKGATE;
    __REG_CLR(HW_AUDIOIN_ANACLKCTRL) = HW_AUDIOIN_ANACLKCTRL__CLKGATE;
    /* set the diethering. they say it improves quality */
    __REG_CLR(HW_AUDIOIN_ANACLKCTRL) = HW_AUDIOIN_ANACLKCTRL__DITHER_OFF;
    /* Hold HP to ground to avoid pop, then release and power up stuff */
    __REG_SET(HW_AUDIOOUT_ANACTRL) = HW_AUDIOOUT_ANACTRL__HP_HOLD_GND;
    __REG_SET(HW_RTC_PERSISTENT0) = 0x00080000;

    /* Power up DAC, ADC, speaker, and headphones */
    __REG_CLR(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__DAC;
    __REG_CLR(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__ADC;
    __REG_CLR(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__SPEAKER;
    __REG_CLR(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__HEADPHONE;
    /* release HP from ground */
    __REG_CLR(HW_RTC_PERSISTENT0) = 0x00080000;

    /* Set HP mode to AB */
    __REG_SET(HW_AUDIOOUT_ANACTRL) = HW_AUDIOOUT_ANACTRL__HP_CLASSAB;
    /* Stop holding to ground */
    __REG_CLR(HW_AUDIOOUT_ANACTRL) = HW_AUDIOOUT_ANACTRL__HP_HOLD_GND;

    __REG_CLR(HW_AUDIOOUT_HPVOL) = HW_AUDIOOUT_HPVOL__MUTE;
    __REG_CLR(HW_AUDIOOUT_HPVOL) = 0x0000FFFF;

    // write some shit to the data to make sure it's not stuck
    HW_AUDIOOUT_DATA = 0x00000000;
    udelay(200);
    HW_AUDIOOUT_DATA = 0x00000001;
    udelay(200);
}

static void audio_dma_init(void)
{
    int i;

    imx233_dma_init();

    for (i=0; i<NUM_PERIODS; i++) {
        g_audio_rec_cmds[i].dma.next = (struct apb_dma_command_t*)&g_audio_rec_cmds[(i+1)%NUM_PERIODS];
        g_audio_rec_cmds[i].dma.buffer = (void*)&g_rec_buff[i][0];
        g_audio_rec_cmds[i].dma.cmd = HW_APB_CHx_CMD__COMMAND__WRITE |
            HW_APB_CHx_CMD__CHAIN |
            HW_APB_CHx_CMD__IRQONCMPLT |
            (NUM_SAMPLES*8) << HW_APB_CHx_CMD__XFER_COUNT_BP;
    }

    for (i=0; i<NUM_PERIODS; i++) {
        g_audio_play_cmds[i].dma.next = (struct apb_dma_command_t*)&g_audio_play_cmds[(i+1)%NUM_PERIODS];
        g_audio_play_cmds[i].dma.buffer = (void*)&g_play_buff[(i+1)%NUM_PERIODS][0];
        g_audio_play_cmds[i].dma.cmd = HW_APB_CHx_CMD__COMMAND__READ |
            HW_APB_CHx_CMD__CHAIN |
            HW_APB_CHx_CMD__IRQONCMPLT |
            (NUM_SAMPLES*8) << HW_APB_CHx_CMD__XFER_COUNT_BP;
    }
    
    imx233_dma_reset_channel(APB_AUDIO_DAC);
    imx233_dma_reset_channel(APB_AUDIO_ADC);

    //
    // for some reason, the interrupt handler pitch is 8 bytes
    // instead of 4 bytes (as it should be on startup
    // according to the imx233 manual)
    //

    // clear all the interrupt handlers
    for (i=0; i<256; i++) {
        isr_table[i] = 0;
    }

    // and set our own interrupt handlers
    isr_table[INT_SRC_DAC_DMA*2] = dac_dma_interrupt;
    isr_table[INT_SRC_DAC_ERROR*2] = dac_error_interrupt;
    isr_table[INT_SRC_ADC_DMA*2] = adc_dma_interrupt;
    isr_table[INT_SRC_ADC_ERROR*2] = adc_error_interrupt;

    imx233_icoll_enable_interrupt(INT_SRC_DAC_DMA, true);
    imx233_icoll_enable_interrupt(INT_SRC_ADC_DMA, true);
    imx233_icoll_enable_interrupt(INT_SRC_DAC_ERROR, true);
    imx233_icoll_enable_interrupt(INT_SRC_ADC_ERROR, true);

    imx233_dma_enable_channel_interrupt(APB_AUDIO_ADC, true);
    imx233_dma_enable_channel_interrupt(APB_AUDIO_DAC, true);
}


#define LRADC_CHANNEL 4
#define LRADC_DELAY_INDEX 0
static void lradc_init(void)
{
    imx233_lradc_init();
    imx233_lradc_setup_channel(LRADC_CHANNEL, 1, 0, 0, LRADC_CHANNEL);
    imx233_lradc_enable_channel_irq(LRADC_CHANNEL, false);
    imx233_lradc_clear_channel_irq(LRADC_CHANNEL);
    imx233_lradc_setup_delay(LRADC_DELAY_INDEX, 1<<LRADC_CHANNEL, 1<<LRADC_DELAY_INDEX, 0, 100);
    imx233_lradc_kick_delay(LRADC_DELAY_INDEX);
}


u32 fx_main()
{
    bool int_enabled;
    unsigned int proc_mode;
    char* addr_start;
    unsigned int addr_offset;
    unsigned int tmp;

    int hwver = 9; /* ??? */
    HW_RTC_PERSISTENT4 = hwver;

	/* Turn off auto-slow and other tricks */
    __REG_CLR(HW_CLKCTRL_HBUS) = 0x07f00000U;
    HW_CLKCTRL_HBUS = 0x00000002; /* clock divider for HCLK */

    /* power up the clocks */
    imx233_reset_block(&HW_RTC_CTRL);
    __REG_SET(HW_RTC_PERSISTENT0) = 
        HW_RTC_PERSISTENT0__XTAL32KHZ_PWRUP |
        HW_RTC_PERSISTENT0__XTAL24MHZ_PWRUP |
        HW_RTC_PERSISTENT0__CLOCKSOURCE;

    //serial_puts("hwver is: ");
    //serial_puthex(hwver);
    //serial_puts("\n");

    g_print_cnt = 0;
    g_rec_index = 0;
    g_first_interrupt = true;

    imx233_icoll_init();
    lradc_init();
    audio_dma_init();
    audio_regs_init();

    /* enable interrupts (ARM specific) */
    enable_irq();
    //enable_fiq();

    serial_puts("initialisations complete\n");

    __REG_SET(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__RUN;
    imx233_dma_start_command(APB_AUDIO_DAC, (struct apb_dma_command_t*)&g_audio_play_cmds[0]);
    __REG_SET(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__RUN;
    imx233_dma_start_command(APB_AUDIO_ADC, (struct apb_dma_command_t*)&g_audio_rec_cmds[0]);

    /* 
    proc_mode = get_processor_mode();
    int_enabled = irq_enabled();
    serial_puts("processor mode: ");
    serial_puthex(proc_mode);
    serial_puts(", irqs ");
    serial_puts(int_enabled ? "enabled\n" : "disabled\n");

    serial_puts("\nException vectors:\n");
    serial_hexdump(0xffff0000, 0x40);

    serial_puts("\n\nAUDIOIN regs:\n");
    serial_hexdump(0x8004C000, 0x100);

    serial_puts("\n\nAUDIOOUT regs:\n");
    serial_hexdump(0x80048000, 0x100);
    */

    /*
    serial_puts("\nISR Table is ");
    serial_puthex((void*)isr_table);
    serial_puts(" with contents:\n");
    serial_hexdump(isr_table, 0x100);
    */

    /*
    serial_puts("\n\nRTC regs:\n");
    serial_hexdump(0x8005C000, 0x100);

    serial_puts("\n\nICOLL regs:\n");
    serial_hexdump(0x80000000, 0x100);
    */

    serial_puts("\n");

    while(1) {
        tmp = imx233_lradc_read_channel(LRADC_CHANNEL);
        serial_puts("lradc data: ");
        serial_puthex(tmp);
        serial_puts("\n");

        /*
        tmp = HW_AUDIOIN_DATA;
        serial_puts("ADC data: ");
        serial_puthex(tmp);
        serial_puts("\n");

        tmp = HW_AUDIOOUT_DATA;
        serial_puts("DAC data: ");
        serial_puthex(tmp);
        serial_puts("\n");
        */

        udelay(500000);
    }
    return 0;
}


static void dac_dma_interrupt()
{
    if (HW_APBX_CTRL1 &
        HW_APBX_CTRL1__CHx_CMDCMPLT_IRQ(APB_GET_DMA_CHANNEL(APB_AUDIO_DAC))) {
        imx233_dma_clear_channel_interrupt(APB_AUDIO_DAC);
    }
    else {
        serial_puts("Unknown DAC DMA interrupt\n");
    }

    stmp378x_ack_irq(INT_SRC_DAC_DMA);
}

static void adc_dma_interrupt()
{
    int i;
    int min, max, curr;

    if (HW_APBX_CTRL1 &
        HW_APBX_CTRL1__CHx_CMDCMPLT_IRQ(APB_GET_DMA_CHANNEL(APB_AUDIO_ADC))) {

        imx233_dma_clear_channel_interrupt(APB_AUDIO_ADC);

        if (g_first_interrupt) {
            g_first_interrupt = false;
            __REG_CLR(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__OFFSET_ENABLE;
        }

        max = -0x7fffffff;
        min = 0x7fffffff;

        for (i=0; i<NUM_SAMPLES*2; i+=2) {
            curr = g_rec_buff[g_rec_index][i];
            if (curr > max) max=curr;
            if (curr < min) min=curr;
        }
        if (g_rec_index == 0) {
            g_print_cnt++;
            if ((g_print_cnt & 0x1f) == 0) {
                serial_puts("recording period elapsed. min=");
                serial_puthex(min);
                serial_puts(", max=");
                serial_puthex(max);
                serial_puts("\n");
            }
        }

        // copy the buffer
        for (i=0; i<NUM_SAMPLES*2; ++i) {
            g_play_buff[g_rec_index][i] = g_rec_buff[g_rec_index][i];
        }

        g_rec_index = (g_rec_index+1)%NUM_PERIODS;
    }
    else {
        serial_puts("Unknown ADC DMA interrupt\n");
    }

    stmp378x_ack_irq(INT_SRC_ADC_DMA);
}

static void dac_error_interrupt()
{
    stmp378x_ack_irq(INT_SRC_DAC_ERROR);

    if (HW_AUDIOOUT_CTRL & HW_AUDIOOUT_CTRL__FIFO_UNDERFLOW_IRQ) {
        serial_puts("AUDIOOUT underflow detected\n");
        __REG_CLR(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__FIFO_UNDERFLOW_IRQ;
    }
    else
        serial_puts("Unknown AUDIOOUT error interrupt\n");
}

static void adc_error_interrupt()
{
    stmp378x_ack_irq(INT_SRC_ADC_ERROR);

    if (HW_AUDIOIN_CTRL & HW_AUDIOIN_CTRL__FIFO_OVERFLOW_IRQ) {
        serial_puts("AUDIOIN overflow detected\n");
        __REG_CLR(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__FIFO_OVERFLOW_IRQ;
    }
    else
        serial_puts("Unknown AUDIOIN error interrupt\n");
}

