/* 
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program  is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */


#include "stdint.h"
#include "serial.h"
#include "lradc.h"
#include "system.h"
#include "audio_dma.h"
#include "effects/parameters.h"


/*
 * low pass filter - we have the previous results and the deltas
 * (this enables us to do resonance as well)
 */
static int g_low_pass_prev_result[NUM_CHANNELS];
static int g_low_pass_prev_delta[NUM_CHANNELS];

/*
 * high pass - we have the previous cleans and the previous results
 */
static int g_high_pass_prev_result[NUM_CHANNELS];
static int g_high_pass_prev_clean[NUM_CHANNELS];

/*
 * flanger - we're keeping the current phase
 */
static unsigned int g_flanger_phase[NUM_CHANNELS];

/*
 * tremolo - a semi-phase
 */
static unsigned int g_tremolo_phase[NUM_CHANNELS];


/* TODO: re-design the LRADC channels thing */
#define LRADC_CHANNEL 4

/* for debugging - remove later */
static uint8_t g_print_cnt;


static void modify_buffers(
    int out_buff[],
    int in_buff[],
    unsigned int num_samples,
    unsigned int num_channels
)
{
    int i, j, index, sample, tmp;
    int min, max, curr;

    // TODO: remove this min-max computation and print

    max = -0x7fffffff;
    min = 0x7fffffff;

    for (i=0; i<num_samples; i+=2) {
        curr = in_buff[i];
        if (curr > max) max=curr;
        if (curr < min) min=curr;
    }
    g_print_cnt++;
    if ((g_print_cnt & 0x7f) == 0) {
        serial_puts("min=");
        serial_puthex(min);
        serial_puts(", max=");
        serial_puthex(max);
        serial_puts("\n");
    }

    // start modifying!
    for (i=0; i < num_samples; ++i) {
        for (j=0; j < num_channels; j++) {

            /* some definitions first */
            index = i*num_channels + j;
            sample = in_buff[index] / 0x200; /* 23-bit is enough */

            /*
             * low-pass first, high-pass next.
             * low-pass result is high-pass clean
            */
            g_high_pass_prev_clean[j] = g_low_pass_prev_result[j];

            /* low-pass with resonance */
            g_low_pass_prev_delta[j] *= g_resonance_level[j];
            g_low_pass_prev_delta[j] /= 0x100;
            tmp = sample - g_low_pass_prev_result[j];
            g_low_pass_prev_delta[j] +=
                (sample - g_low_pass_prev_result[j]) *
                g_low_pass_level[j] / 0x100;
            g_low_pass_prev_delta[j] = limit_value_of_delta(g_low_pass_prev_delta[j]);
            sample = limit_value_of_sample(
                g_low_pass_prev_result[j] + g_low_pass_prev_delta[j]
            );
            g_low_pass_prev_result[j] = sample;

            /* high-pass */
            sample = limit_value_of_sample(
                (g_high_pass_prev_result[j] + sample-g_high_pass_prev_clean[j]) *
                g_high_pass_level[j] / 0x100
            );
            g_high_pass_prev_result[j] = sample;
        }
    }
}

int fx_main()
{
    unsigned int tmp;

    system_init();
    lradc_setup_channel_for_polling(LRADC_CHANNEL);
    audio_setup();
    audio_dma_init(modify_buffers);

    g_print_cnt = 0;

    serial_puts("initialisations complete\n");

    audio_dma_start();

    serial_puts("\n");

    while(1) {
        tmp = lradc_read_channel(LRADC_CHANNEL);
        serial_puts("lradc data: ");
        serial_puthex(tmp);
        serial_puts("\n");

        udelay(500000);
    }
    return 0;
}

