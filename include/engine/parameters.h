#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__


#include "lradc.h"
#include "metronome.h"


#define NUM_CHANNELS 2


/*
 * each parameter has a variable that describes
   how it is controlled:
 ** MANUAL - level controlled by a potentiometer
        (other variables describe which potentiometer)
 ** LFO - level controlled by a low-frequency oscillator
        with one potentiometer that controls the frequency
        and another potentiometer that controls the level
 ** METRONOME - with metronome operators (see metronome.h and metronome.c)
 ** EXTERNAL - level is set by an external force (UART/USB/MIDI/aliens)
 ** FIXED - don't touch this!
 */
typedef enum _param_ctrl_t {
    PARAM_CTRL_MANUAL,
    PARAM_CTRL_LFO,
    PARAM_CTRL_METRONOME,
    PARAM_CTRL_EXTERNAL,
    PARAM_CTRL_FIXED,
    PARAM_CTRL_MAX
} param_ctrl_t;


typedef enum _lfo_op_t {
    LFO_OP_LINEAR,
    LFO_OP_SINE,
    LFO_OP_EXP,
    LFO_OP_MAX
} lfo_op_t;


void parameters_setup();
void parameters_set();
void parameters_counter_increment();


/*
 * phase_perform_op
 *
 * return the computed phase based on the operator op,
 * on the current level and the next level, and on the phase.
 *
 * the purpose of this function is to make a smooth transition
 * between curr_level and next_level.
 *
 * phase (between 0 and 1) determines how close we should be
 * to either one of the edges.
 *
 * op determines the transition factor (linear, exponential, etc.)
 */
double phase_perform_op(
    const metronome_op_t op,
    const double phase,
    const double curr_level,
    const double next_level
);


double lfo_perform_op(
    const lfo_op_t op,
    const double phase
);



#endif /* __PARAMETERS_H__ */

