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
#include "gpio.h"
#include "math.h"
#include "utils/str.h"
#include "engine/parameters.h"
#include "engine/metronome.h"
#include "effects/effect_base.h"
#include "effects/resonance.h"
#include "effects/low_pass.h"


#ifdef __cplusplus
extern "C" {
int fx_main();
}
#endif


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
    unsigned int i, j, k, index;

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

            for (k=0; k<MAX_EFFECT_COUNT; k++) {
                if (g_effects[k] == NULL) break;
                sample = g_effects[k]->process_sample(sample, j);
            }

            out_buff[index] = sample * 0x200; /* scale back from 23-bit to 32 */
        }

        // take care of other parameters too
        parameters_counter_increment();
    }
}

int fx_main()
{
    system_init();
    audio_setup();
    audio_dma_init(modify_buffers);

    gpio_setup();

    g_print_cnt = 0;

    parameters_setup();

    serial_puts("initialisations complete\n");

    audio_dma_start();

    // start the metronome
    metronome_start();

    while(1) {
        parameters_set();
        udelay(50000);
    }
    return 0;
}

