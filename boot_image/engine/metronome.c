#include "engine/metronome.h"
#include "engine/parameters.h"


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
/* array of operations to perform for each fraction */
static metronome_op_t
    g_ops[OP_TYPE_MAX][MAX_DIVISION_FACTOR * MAX_PATTERN_UNITS];
static unsigned short
    g_levels[OP_TYPE_MAX][MAX_DIVISION_FACTOR * MAX_PATTERN_UNITS];


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
    int i;

    g_f_active = 0;

    /* TODO: limit (and check correctness of) the following values */
    g_bpm = freq;
    g_num_ops = num_ops;
    g_pattern_units = pattern_units;

    for (i = 0; i < MAX_DIVISION_FACTOR * MAX_PATTERN_UNITS; i++) {
        g_levels[OP_TYPE_VOLUME][i] = VOLUME_NORMAL_LEVEL;
        g_levels[OP_TYPE_LOW_PASS][i] = LOW_PASS_MAX_LEVEL;
        g_levels[OP_TYPE_RESONANCE][i] = RESONANCE_NORMAL_LEVEL;
        g_levels[OP_TYPE_HIGH_PASS][i] = HIGH_PASS_MAX_LEVEL;
        g_levels[OP_TYPE_FLANGER][i] = FLANGER_NORMAL_MIX_LEVEL;
        g_levels[OP_TYPE_DISTORTION][i] = DISTORTION_NORMAL_LEVEL;
    }
    
    /* TODO: setup the LEDs and clear them */
}

void metronome_set_ops(
    metronome_op_type_t type,
    metronome_op_t ops[],
    unsigned short levels[]
)
{
    int i;

    for (i=0; i<g_num_ops*g_pattern_units; i++) {
        g_ops[type][i] = ops[i];
        g_levels[type][i] = levels[i];
    }
}

void metronome_start()
{
    if (!g_f_active) {
        g_phase_cnt = 0;
        g_phase = 0;
        g_op_index = 0;
        g_unit_index = 0;

        g_f_active = 1;

        /* TODO: turn on the primary LED */
        /* TODO: set parameters according to the first pattern ops */
    }
}

void metronome_stop()
{
    g_f_active = 0;
    /* TODO: turn off the LEDs */
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
                    /* TODO: set the primary metronome led */
                }
                else {
                    /* TODO: set the secondary metronome led */
                }
            }
        }

        /* TODO: set the parameters according to the phase and the op */
        /* test - set the low-pass to change linearly from one to zero */
        for (j=0; j<NUM_CHANNELS; j++) {
            g_low_pass_level[j] = two_exp_12bit_to_8bit((0x100-g_phase)*16);
        }
    }
}

