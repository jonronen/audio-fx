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
 ** METRONOME_WITH_MANUAL_LEVEL - metronome operators
        with manual level control
 ** EXTERNAL - level is set by an external force (UART/USB/MIDI/aliens)
 ** FIXED - don't touch this!
 */
typedef enum _param_ctrl_t {
    PARAM_CTRL_MANUAL,
    PARAM_CTRL_LFO,
    PARAM_CTRL_METRONOME,
    PARAM_CTRL_METRONOME_WITH_MANUAL_LEVEL,
    PARAM_CTRL_EXTERNAL,
    PARAM_CTRL_FIXED,
    PARAM_CTRL_MAX
} param_ctrl_t;



void parameters_setup();
void parameters_set();
void parameters_counter_increment();



/* parameter manipulation */
unsigned short two_exp_12bit_to_8bit(unsigned short raw_value);

/*
 * phases translation functions
 * input: 8-bit phase
 * output: 12-bit level (zero means no effect, 0x1000 means full power)
 */
unsigned short phase_to_sine_wave(unsigned char phase);
unsigned short phase_to_sawtooth_wave(unsigned char phase);
unsigned short phase_to_reverse_sawtooth_wave(unsigned char phase);
unsigned short phase_to_triangular_wave(unsigned char phase);
unsigned short phase_to_square_wave(unsigned char phase);
unsigned short phase_perform_op(metronome_op_t op, unsigned char phase);



#endif /* __PARAMETERS_H__ */

