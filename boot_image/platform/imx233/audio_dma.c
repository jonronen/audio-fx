#include "serial.h"
#include "system.h"
#include "audio_dma.h"
#include "platform/imx233/rtc.h"
#include "platform/imx233/dma.h"
#include "platform/imx233/audioin.h"
#include "platform/imx233/audioout.h"


static uint8_t g_rec_index;
static audio_dma_callback_t g_callback;


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


void audio_setup()
{
    imx233_reset_block(&HW_AUDIOIN_CTRL);
    imx233_reset_block(&HW_AUDIOOUT_CTRL);
    /* remove the gate-off for DIGFILT */
    __REG_CLR(HW_CLKCTRL_XTAL) = 0x40000000;

    /* Set the audio recorder to use LRADC1, and set to an 8K resistor. */
    //__REG_SET(HW_AUDIOIN_MICLINE) = 0x00000001;

    __REG_CLR(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__FIFO_OVERFLOW_IRQ;
    __REG_CLR(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__FIFO_UNDERFLOW_IRQ;
    //__REG_SET(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__FIFO_ERROR_IRQ_EN;
    //__REG_SET(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__OFFSET_ENABLE;
    //__REG_SET(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__HPF_ENABLE;

    __REG_CLR(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__FIFO_OVERFLOW_IRQ;
    __REG_CLR(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__FIFO_UNDERFLOW_IRQ;
    //__REG_SET(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__FIFO_ERROR_IRQ_EN;

    // set the frequencies to 44.1KHz
    HW_AUDIOIN_ADCSRR = 0x10110037;
    HW_AUDIOOUT_DACSRR = 0x10110037;

    //__REG_SET(HW_AUDIOIN_CTRL) = 0x001f0000; // TODO: change that?
    //__REG_SET(HW_AUDIOOUT_CTRL) = 0x001f0000; // TODO: change that?
    HW_AUDIOIN_ADCVOL = 0x00001111; // take the audio from LINE1
    HW_AUDIOIN_ADCVOLUME = 0x00fe00fe;
    HW_AUDIOOUT_DACVOLUME = 0x00ff00ff;

    /* Set word-length to 32-bit */
    __REG_CLR(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__WORD_LENGTH;
    __REG_CLR(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__WORD_LENGTH;
    /* Enable DAC and ADC */
    __REG_CLR(HW_AUDIOOUT_ANACLKCTRL) = HW_AUDIOOUT_ANACLKCTRL__CLKGATE;
    __REG_CLR(HW_AUDIOIN_ANACLKCTRL) = HW_AUDIOIN_ANACLKCTRL__CLKGATE;
    /* Hold HP to ground to avoid pop, then release and power up stuff */
    __REG_SET(HW_AUDIOOUT_ANACTRL) = HW_AUDIOOUT_ANACTRL__HP_HOLD_GND;
    __REG_SET(HW_RTC_PERSISTENT0) = 0x00080000;

    /* Power up DAC, ADC, speaker, and headphones */
    __REG_CLR(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__DAC;
    __REG_CLR(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__ADC;
    __REG_SET(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__SPEAKER; // spkr is off
    __REG_CLR(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__HEADPHONE;
    /* release HP from ground */
    __REG_CLR(HW_RTC_PERSISTENT0) = 0x00080000;

    /* Set HP mode to AB */
    __REG_SET(HW_AUDIOOUT_ANACTRL) = HW_AUDIOOUT_ANACTRL__HP_CLASSAB;
    /* Stop holding to ground */
    __REG_CLR(HW_AUDIOOUT_ANACTRL) = HW_AUDIOOUT_ANACTRL__HP_HOLD_GND;

    __REG_CLR(HW_AUDIOOUT_HPVOL) = HW_AUDIOOUT_HPVOL__MUTE;
    __REG_CLR(HW_AUDIOOUT_HPVOL) = 0x00007f7f;
    //__REG_SET(HW_AUDIOOUT_HPVOL) = 0x00014040; // test only - line1->HP
    __REG_SET(HW_AUDIOOUT_HPVOL) = 0x00008080; // volume shouldn't be too high

    // write some shit to the data to make sure it's not stuck
    HW_AUDIOOUT_DATA = 0x00000000;
    udelay(200);
    HW_AUDIOOUT_DATA = 0x00000001;
    udelay(200);
}

void audio_dma_init(audio_dma_callback_t p_callback)
{
    int i;

    g_rec_index = 0;
    g_callback = p_callback;

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

    // and set our own interrupt handlers
    imx233_icoll_set_handler(INT_SRC_DAC_DMA, dac_dma_interrupt);
    imx233_icoll_set_handler(INT_SRC_DAC_ERROR, dac_error_interrupt);
    imx233_icoll_set_handler(INT_SRC_ADC_DMA, adc_dma_interrupt);
    imx233_icoll_set_handler(INT_SRC_ADC_ERROR, adc_error_interrupt);

    imx233_icoll_enable_interrupt(INT_SRC_DAC_DMA, true);
    imx233_icoll_enable_interrupt(INT_SRC_ADC_DMA, true);
    imx233_icoll_enable_interrupt(INT_SRC_DAC_ERROR, true);
    imx233_icoll_enable_interrupt(INT_SRC_ADC_ERROR, true);

    imx233_dma_enable_channel_interrupt(APB_AUDIO_ADC, true);
    imx233_dma_enable_channel_interrupt(APB_AUDIO_DAC, true);
}


void audio_dma_start()
{
    /* enable interrupts first */
    enable_irq();

    /* then start the DAC and ADC modules with their proper DMA commands */
    __REG_SET(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__RUN;
    imx233_dma_start_command(APB_AUDIO_DAC, (struct apb_dma_command_t*)&g_audio_play_cmds[0]);
    __REG_SET(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__RUN;
    imx233_dma_start_command(APB_AUDIO_ADC, (struct apb_dma_command_t*)&g_audio_rec_cmds[0]);
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

    imx233_icoll_ack_irq(INT_SRC_DAC_DMA);
}

static void adc_dma_interrupt()
{
    if (HW_APBX_CTRL1 &
        HW_APBX_CTRL1__CHx_CMDCMPLT_IRQ(APB_GET_DMA_CHANNEL(APB_AUDIO_ADC))) {

        imx233_dma_clear_channel_interrupt(APB_AUDIO_ADC);

        // handle the buffers
        g_callback(
            g_play_buff[g_rec_index],
            g_rec_buff[g_rec_index],
            NUM_SAMPLES,
            2 /* two channels for each sample */
        );

        g_rec_index = (g_rec_index+1)%NUM_PERIODS;
    }
    else {
        serial_puts("Unknown ADC DMA interrupt\n");
    }

    imx233_icoll_ack_irq(INT_SRC_ADC_DMA);
}

static void dac_error_interrupt()
{
    imx233_icoll_ack_irq(INT_SRC_DAC_ERROR);

    if (HW_AUDIOOUT_CTRL & HW_AUDIOOUT_CTRL__FIFO_UNDERFLOW_IRQ) {
        serial_puts("AUDIOOUT underflow detected\n");
        __REG_CLR(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__FIFO_UNDERFLOW_IRQ;
    }
    else
        serial_puts("Unknown AUDIOOUT error interrupt\n");
}

static void adc_error_interrupt()
{
    imx233_icoll_ack_irq(INT_SRC_ADC_ERROR);

    if (HW_AUDIOIN_CTRL & HW_AUDIOIN_CTRL__FIFO_OVERFLOW_IRQ) {
        serial_puts("AUDIOIN overflow detected\n");
        __REG_CLR(HW_AUDIOIN_CTRL) = HW_AUDIOIN_CTRL__FIFO_OVERFLOW_IRQ;
    }
    else
        serial_puts("Unknown AUDIOIN error interrupt\n");
}

