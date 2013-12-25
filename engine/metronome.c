#include "stdint.h"
#include "engine/metronome.h"
#include "engine/parameters.h"
#include "effects/effect_base.h"
#include "gpio.h"


#define PHASES_PER_OP 256



/*
 * "fixed" parameters
 */

/* beats per minute. assuming between 40 and 300 */
static unsigned short g_bpm;
/* number of fractions per beat. assuming between 1 and MAX_DIVISION_FACTOR */
static unsigned char g_num_ops;
/* number of beats per pattern. assuming between 1 and 16 */
static unsigned char g_pattern_units;


/*
 * variables
 */
static unsigned int g_phase_cnt;
static unsigned char g_phase;
static unsigned char g_op_index;
static unsigned char g_unit_index;
static unsigned int g_f_active;

void metronome_setup(
    unsigned short freq,
    unsigned char num_ops,
    unsigned char pattern_units
)
{
    g_f_active = 0;

    /* TODO: limit (and check correctness of) the following values */
    g_bpm = freq;
    g_num_ops = num_ops;
    g_pattern_units = pattern_units;
}

void metronome_start()
{
    if (!g_f_active) {
        g_phase_cnt = 0;
        g_phase = 0;
        g_op_index = 0;
        g_unit_index = 0;

        g_f_active = 1;

        gpio_set_metronome_output(true, true);

        /* TODO: set parameters according to the first pattern ops */
    }
}

void metronome_stop()
{
    g_f_active = 0;
    gpio_set_metronome_output(true, false);
    gpio_set_metronome_output(false, false);
}

void metronome_tick()
{
    int j;

    if (!g_f_active) return;

    g_phase_cnt += (g_bpm * g_num_ops * PHASES_PER_OP);
    if (g_phase_cnt >= (TICK_FREQUENCY*BPMS_PER_SECOND)) {
        g_phase_cnt -= (TICK_FREQUENCY*BPMS_PER_SECOND);
        g_phase++;
        if (g_phase == 0) {
            g_op_index++;
            if (g_op_index >= g_num_ops) {
                g_op_index = 0;
                g_unit_index++;
                if (g_unit_index >= g_pattern_units) {
                    g_unit_index = 0;
                    gpio_set_metronome_output(true, true);
                }
                else {
                    gpio_set_metronome_output(false, true);
                }
            }
        }
        else if (g_phase == 0x80) {
            gpio_set_metronome_output(true, false);
            gpio_set_metronome_output(false, false);
        }

        /* set the parameters according to the phase and the op */
        for (j=0; j<NUM_CHANNELS; j++) {
            if (g_effects[j] == NULL) break;
            g_effects[j]->metronome_phase(g_phase, g_op_index);
        }
    }
}

