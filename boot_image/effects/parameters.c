#include "effects/parameters.h"
#include "lradc.h"



/*
 * overdrive and distortion
 * TODO: units!
 */
unsigned short g_overdrive_level[NUM_CHANNELS];
unsigned short g_distortion_level[NUM_CHANNELS];


/*
 * low-pass level
 * TODO: units!
 */
unsigned short g_low_pass_level[NUM_CHANNELS];

/*
 * resonance (goes together with low-pass filter)
 */
unsigned short g_resonance_level[NUM_CHANNELS];


/*
 * high-pass level
 * TODO: units!
 */
unsigned short g_high_pass_level[NUM_CHANNELS];


/*
 * tremolo
 */
unsigned short g_tremolo_frequency[NUM_CHANNELS];
unsigned short g_tremolo_level[NUM_CHANNELS];


/*
 * flanger
 */
unsigned short g_flanger_low_freq_limit[NUM_CHANNELS];
unsigned short g_flanger_high_freq_limit[NUM_CHANNELS];
unsigned short g_flanger_frequency[NUM_CHANNELS];
unsigned short g_flanger_mix_level[NUM_CHANNELS];



#define LRADC_CHANNEL 4


void parameters_setup()
{
    int j;

    /* clear all the parameters at start */
    for (j=0; j<NUM_CHANNELS; j++) {
        g_overdrive_level[j] = 0x100;
        g_distortion_level[j] = 0;
        g_low_pass_level[j] = 0x100;
        g_resonance_level[j] = 0;
        g_high_pass_level[j] = 0x100;
        g_tremolo_frequency[j] = 1;
        g_tremolo_level[j] = 0;
        g_flanger_low_freq_limit[j] = 1;
        g_flanger_high_freq_limit[j] = 1;
        g_flanger_frequency[j] = 1;
        g_flanger_mix_level[j] = 0;
    }

    lradc_setup_channel_for_polling(LRADC_CHANNEL);
}


void parameters_set()
{
    unsigned int tmp;
    int j;

    /* test - remove later */
    tmp = lradc_read_channel(LRADC_CHANNEL);
    for (j = 0; j < NUM_CHANNELS; j++) {
        g_low_pass_level[j] = tmp / 4;
    }
}

