#include "stdint.h"
#include "stddef.h"
#include "engine/metronome.h"
#include "engine/parameters.h"
#include "effects/effect_base.h"
#include "gpio.h"



/*
 * "fixed" parameters
 */

/* beats per minute. assuming between 40 and 300 */
static unsigned short g_bpm;
/* number of fractions per beat. assuming between 1 and MAX_DIVISION_FACTOR */
static unsigned char g_ops_per_beat;
/* number of beats per pattern. assuming between 1 and MAX_PATTERN_UNITS */
static unsigned char g_beat_count;


/*
 * variables
 */
static unsigned int g_phase_cnt;
static unsigned char g_half_unit_count;
static unsigned char g_op_index;
static unsigned char g_beat_index;
static unsigned int g_f_active;

void metronome_setup(
    unsigned short freq,
    unsigned char ops_per_beat,
    unsigned char beat_count
)
{
    g_f_active = 0;

    /* TODO: limit (and check correctness of) the following values */
    g_bpm = freq;
    g_ops_per_beat = ops_per_beat;
    g_beat_count = beat_count;
}

void metronome_start()
{
    if (!g_f_active) {
        g_phase_cnt = 0;
        g_half_unit_count = 0;
        g_op_index = 0;
        g_beat_index = 0;

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
    double metronome_phase = 0.0;

    if (!g_f_active) return;

    g_phase_cnt += (g_bpm * g_ops_per_beat * 2);
    if (g_phase_cnt >= (TICK_FREQUENCY*BPMS_PER_SECOND)) {
        g_phase_cnt -= (TICK_FREQUENCY*BPMS_PER_SECOND);
        g_half_unit_count++;
        if (g_half_unit_count == 2) {
            g_half_unit_count = 0;
            g_op_index++;
            if (g_op_index >= g_ops_per_beat) {
                g_op_index = 0;
                g_beat_index++;
                if (g_beat_index >= g_beat_count) {
                    g_beat_index = 0;
                    gpio_set_metronome_output(true, true);
                }
                else {
                    gpio_set_metronome_output(false, true);
                }
            }
        }
        else { /* g_half_unit_count = 1 */
            gpio_set_metronome_output(true, false);
            gpio_set_metronome_output(false, false);
        }

        metronome_phase = (double)g_phase_cnt /
            (TICK_FREQUENCY*BPMS_PER_SECOND);
        metronome_phase += (double)0.5 * (double)g_half_unit_count;

        /* set the parameters according to the phase and the op */
        for (j=0; j<NUM_CHANNELS; j++) {
            if (g_effects[g_preset_count][j] == NULL) break;
            g_effects[g_preset_count][j]->metronome_phase(
                metronome_phase,
                g_beat_index * g_ops_per_beat + g_op_index
            );
        }
    }
}

