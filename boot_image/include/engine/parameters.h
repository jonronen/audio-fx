#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__


#include "lradc.h"


#define NUM_CHANNELS 2


/* allow up to 64 effects in parallel */
#define MAX_EFFECT_COUNT 64


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



/*
 * overdrive and distortion
 */

/*
 * overdrive units are between 0x40 (no overdrive)
 * and 0x100 (multiply by four).
 * technically, it can be less than 0x40,
 * but this is not the intention of this effect.
 */
extern unsigned short g_overdrive_level[NUM_CHANNELS];
#define OVERDRIVE_NORMAL_LEVEL 0x40

/* TODO: distortion units */
extern unsigned short g_distortion_level[NUM_CHANNELS];
#define DISTORTION_NORMAL_LEVEL 1


/*
 * low-pass level
 *
 * units are from zero (nothing passes) to 0x100 (all-pass)
 * TODO: scale the units properly (exponential?)
 */
extern unsigned short g_low_pass_level[NUM_CHANNELS];
#define LOW_PASS_MAX_LEVEL 0x100

/*
 * resonance (goes together with low-pass filter)
 * units are from zero (no resonance) to 0x100 (TODO: consider more)
 */
extern unsigned short g_resonance_level[NUM_CHANNELS];
#define RESONANCE_MAX_LEVEL 0x100
#define RESONANCE_NORMAL_LEVEL 0


/*
 * high-pass level
 *
 * units are from zero (nothing passes) to 0x100 (all-pass)
 * TODO: scale the units properly (exponential?)
 */
extern unsigned short g_high_pass_level[NUM_CHANNELS];
#define HIGH_PASS_MAX_LEVEL 0x100


/*
 * volume
 *
 * units are between zero (mute) and 0x100 (no change).
 * the intention of this value is to implement effects like tremolo,
 * fade-in, fade-out, etc.
 *
 * do not use this to turn up the volume. use overdrive instead.
 */
extern unsigned short g_volume_factor[NUM_CHANNELS];
#define VOLUME_NORMAL_LEVEL 0x100


/*
 * flanger
 */
extern unsigned short g_flanger_low_freq_limit[NUM_CHANNELS];
extern unsigned short g_flanger_high_freq_limit[NUM_CHANNELS];
extern unsigned short g_flanger_frequency[NUM_CHANNELS];
extern unsigned short g_flanger_mix_level[NUM_CHANNELS];
#define FLANGER_NORMAL_MIX_LEVEL 0



void parameters_setup();
void parameters_set();
void parameters_counter_increment();



/* parameter manipulation */
unsigned short two_exp_12bit_to_8bit(unsigned short raw_value);
unsigned short phase_to_sine_wave(unsigned char phase);
unsigned short phase_to_sawtooth_wave(unsigned char phase);
unsigned short phase_to_reverse_sawtooth_wave(unsigned char phase);
unsigned short phase_to_triangular_wave(unsigned char phase);
unsigned short phase_to_square_wave(unsigned char phase);



#endif /* __PARAMETERS_H__ */

