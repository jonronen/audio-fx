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
#include "math.h"
#include "engine/parameters.h"
#include "engine/metronome.h"


#ifdef __cplusplus
extern "C" {
int fx_main();
}
#endif



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


/* for debugging - remove later */
static uint8_t g_print_cnt;


static void modify_buffers(
    int out_buff[],
    int in_buff[],
    unsigned int num_samples,
    unsigned int num_channels
)
{
    int sample;
    unsigned int i, j, index;

    // TODO: remove this min-max computation and print
    int min, max, curr;

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

            /* overdrive */
            sample = limit_value_of_sample(
                sample * (int)g_overdrive_level[j] / OVERDRIVE_NORMAL_LEVEL
            );

            /* distortion (TODO: doesn't work properly...) */
            /*
            sample = (
                (sample + (int)g_distortion_level[j]/2) /
                (int)g_distortion_level[j]
            ) * (int)g_distortion_level[j];
            */

            /*
             * low-pass first, high-pass next.
             * low-pass result is high-pass clean
            */
            g_high_pass_prev_clean[j] = g_low_pass_prev_result[j];

            /* low-pass with resonance */
            g_low_pass_prev_delta[j] *= g_resonance_level[j];
            g_low_pass_prev_delta[j] /= RESONANCE_MAX_LEVEL;
            g_low_pass_prev_delta[j] +=
                (((sample - g_low_pass_prev_result[j]) *
                  (int)g_low_pass_level[j]) / LOW_PASS_MAX_LEVEL);
            g_low_pass_prev_delta[j] = limit_value_of_delta(g_low_pass_prev_delta[j]);
            sample = limit_value_of_sample(
                g_low_pass_prev_result[j] + g_low_pass_prev_delta[j]
            );
            g_low_pass_prev_result[j] = sample;

            /* high-pass */
            sample = limit_value_of_sample(
                (g_high_pass_prev_result[j] + sample-g_high_pass_prev_clean[j]) *
                (int)g_high_pass_level[j] / HIGH_PASS_MAX_LEVEL
            );
            g_high_pass_prev_result[j] = sample;

            /*
             * in the end, do the volume adjustment.
             * don't confuse this with overdrive,
             * this is for effects like tremolo.
             */
            sample = sample * (int)g_volume_factor[j] / VOLUME_NORMAL_LEVEL;

            out_buff[index] = sample * 0x200; /* scale back from 23-bit to 32 */
        }

        // take care of other parameters too
        parameters_counter_increment();
    }
}

int fx_main()
{
    int j;

    system_init();
    audio_setup();
    audio_dma_init(modify_buffers);

    /* g_print_cnt = 0; */

    for (j=0; j<NUM_CHANNELS; j++) {
        g_low_pass_prev_result[j] = 0;
        g_low_pass_prev_delta[j] = 0;
        g_high_pass_prev_result[j] = 0;
        g_high_pass_prev_clean[j] = 0;
        g_flanger_phase[j] = 0;
        g_tremolo_phase[j] = 0;
    }

    parameters_setup();
    metronome_setup(140, 16, 4);

    serial_puts("initialisations complete\n");

    // test - initialise the metronome with some fixed parameters
    metronome_setup(120, 2, 1);

    audio_dma_start();

    // test - start the metronome
    metronome_start();

    while(1) {
        parameters_set();
        udelay(50000);
    }
    return 0;
}

