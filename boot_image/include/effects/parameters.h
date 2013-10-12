#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__


#define NUM_CHANNELS 2


/*
 * overdrive and distortion
 * TODO: units!
 */
extern unsigned short g_overdrive_level[NUM_CHANNELS];
extern unsigned short g_distortion_level[NUM_CHANNELS];


/*
 * low-pass level
 * TODO: units!
 */
extern unsigned short g_low_pass_level[NUM_CHANNELS];

/*
 * resonance (goes together with low-pass filter)
 */
extern unsigned short g_resonance_level[NUM_CHANNELS];


/*
 * high-pass level
 * TODO: units!
 */
extern unsigned short g_high_pass_level[NUM_CHANNELS];


/*
 * tremolo
 */
extern unsigned short g_tremolo_frequency[NUM_CHANNELS];
extern unsigned short g_tremolo_level[NUM_CHANNELS];


/*
 * flanger
 */
extern unsigned short g_flanger_low_freq_limit[NUM_CHANNELS];
extern unsigned short g_flanger_high_freq_limit[NUM_CHANNELS];
extern unsigned short g_flanger_frequency[NUM_CHANNELS];
extern unsigned short g_flanger_mix_level[NUM_CHANNELS];



void parameters_setup();
void parameters_set();



#endif /* __PARAMETERS_H__ */

