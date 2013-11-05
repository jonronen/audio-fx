#include "effects/metronome.h"
#include "effects/parameters.h"


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
static metronome_op_t g_ops[MAX_DIVISION_FACTOR * MAX_PATTERN_UNITS];


/*
 * variables
 */
static unsigned int g_phase_cnt;
static unsigned char g_phase;
static unsigned char g_op_index;
static unsigned char g_unit_index;

void set_frequency(unsigned short freq)
{
    g_bpm = freq;
}

void set_ops(metronome_op_t ops[], unsigned char num_ops)
{
    int i;

    g_num_ops = num_ops;
    for (i=0; i<num_ops; i++) {
        g_ops[i] = ops[i];
    }

    /* TODO: set initial parameters according to the first op in the pattern */
}

void tick()
{
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
    }
}

