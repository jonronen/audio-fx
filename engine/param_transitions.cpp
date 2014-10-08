#include "effects/effect_base.h"
#include "engine/parameters.h"
#include "engine/metronome.h"
#include "lradc.h"
#include "stdint.h"
#include "stddef.h"
#include "serial.h"
#include "utils/str.h"
#include "fx_math.h"


static double phase_sine_transition(
    const double curr_level,
    const double next_level,
    const double phase
)
{
    double factor = scaled_sine(phase);
    double res =
        curr_level * (1.0 - factor) +
        next_level * factor;

    return res;
}


static double phase_exp_transition(
    const double curr_level,
    const double next_level,
    const double phase
)
{
    double factor = phase * phase;
    factor = factor * factor;
    factor = factor * factor;
    double res =
        curr_level * (1-factor) +
        next_level * factor;

    return res;
}


static double phase_linear_transition(
    const double curr_level,
    const double next_level,
    const double phase
)
{
    return (curr_level * (1-phase)) + (next_level * phase);
}


double phase_perform_op(
    metronome_op_t op,
    double phase,
    double curr_level,
    double next_level
)
{
    double res = 0.0;

    switch(op) {
      case METRONOME_OP_CONST_NONE:
        res = 0.0;
        break;
      case METRONOME_OP_CONST_FULL:
        res = curr_level;
        break;
      case METRONOME_OP_LINEAR_RISE:
        res = phase_linear_transition(0, curr_level, phase);
        break;
      case METRONOME_OP_LINEAR_FALL:
        res = phase_linear_transition(curr_level, 0, phase);
        break;
      case METRONOME_OP_LINEAR_TRANSITION:
        res = phase_linear_transition(curr_level, next_level, phase);
        break;
      case METRONOME_OP_SINE_RISE:
        res = phase_sine_transition(0, curr_level, phase);
        break;
      case METRONOME_OP_SINE_FALL:
        res = phase_sine_transition(curr_level, 0, phase);
        break;
      case METRONOME_OP_SINE_TRANSITION:
        res = phase_sine_transition(curr_level, next_level, phase);
        break;
      case METRONOME_OP_EXP_RISE:
        res = phase_exp_transition(0, curr_level, phase);
        break;
      case METRONOME_OP_EXP_FALL:
        res = phase_exp_transition(curr_level, 0, phase);
        break;
      case METRONOME_OP_EXP_TRANSITION:
        res = phase_exp_transition(curr_level, next_level, phase);
        break;
      default:
        res = 0;
    }

    return res;
}


void parameters_set()
{
    int i;

    for (i=0; i<MAX_EFFECT_COUNT; i++) {
        if (g_effects[g_preset_count][i] == NULL) break;
        g_effects[g_preset_count][i]->params_update();
    }
}


void parameters_counter_increment()
{
    int i;

    for (i=0; i<MAX_EFFECT_COUNT; i++) {
        if (g_effects[g_preset_count][i] == NULL) break;
        g_effects[g_preset_count][i]->params_tick();
    }
    metronome_tick();
}

