#include "effects/parameters.h"
#include "lradc.h"



/*
 * overdrive and distortion
 */
unsigned short g_overdrive_level[NUM_CHANNELS];
unsigned short g_distortion_level[NUM_CHANNELS];


/*
 * low-pass level
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
 * volume
 */
unsigned short g_volume_factor[NUM_CHANNELS];


/*
 * flanger
 */
unsigned short g_flanger_low_freq_limit[NUM_CHANNELS];
unsigned short g_flanger_high_freq_limit[NUM_CHANNELS];
unsigned short g_flanger_frequency[NUM_CHANNELS];
unsigned short g_flanger_mix_level[NUM_CHANNELS];



/*
 * numerical tables for parameter conversions (e.g. logarithms)
 */
static const unsigned short g_two_exp_fraction[] = {
    256, 256, 256, 257, 257, 257, 258, 258, 
    258, 259, 259, 259, 260, 260, 260, 261, 
    261, 261, 262, 262, 263, 263, 263, 264, 
    264, 264, 265, 265, 265, 266, 266, 266, 
    267, 267, 268, 268, 268, 269, 269, 269, 
    270, 270, 270, 271, 271, 272, 272, 272, 
    273, 273, 273, 274, 274, 275, 275, 275, 
    276, 276, 276, 277, 277, 278, 278, 278, 
    279, 279, 279, 280, 280, 281, 281, 281, 
    282, 282, 282, 283, 283, 284, 284, 284, 
    285, 285, 286, 286, 286, 287, 287, 287, 
    288, 288, 289, 289, 289, 290, 290, 291, 
    291, 291, 292, 292, 293, 293, 293, 294, 
    294, 295, 295, 295, 296, 296, 297, 297, 
    297, 298, 298, 299, 299, 299, 300, 300, 
    301, 301, 301, 302, 302, 303, 303, 304, 
    304, 304, 305, 305, 306, 306, 306, 307, 
    307, 308, 308, 309, 309, 309, 310, 310, 
    311, 311, 311, 312, 312, 313, 313, 314, 
    314, 314, 315, 315, 316, 316, 317, 317, 
    317, 318, 318, 319, 319, 320, 320, 320, 
    321, 321, 322, 322, 323, 323, 323, 324, 
    324, 325, 325, 326, 326, 327, 327, 327, 
    328, 328, 329, 329, 330, 330, 331, 331, 
    331, 332, 332, 333, 333, 334, 334, 335, 
    335, 336, 336, 336, 337, 337, 338, 338, 
    339, 339, 340, 340, 341, 341, 342, 342, 
    342, 343, 343, 344, 344, 345, 345, 346, 
    346, 347, 347, 348, 348, 349, 349, 349, 
    350, 350, 351, 351, 352, 352, 353, 353, 
    354, 354, 355, 355, 356, 356, 357, 357, 
    358, 358, 359, 359, 360, 360, 361, 361, 
    362, 362, 363, 363, 364, 364, 364, 365, 
    365, 366, 366, 367, 367, 368, 368, 369, 
    369, 370, 370, 371, 371, 372, 372, 373, 
    373, 374, 375, 375, 376, 376, 377, 377, 
    378, 378, 379, 379, 380, 380, 381, 381, 
    382, 382, 383, 383, 384, 384, 385, 385, 
    386, 386, 387, 387, 388, 388, 389, 390, 
    390, 391, 391, 392, 392, 393, 393, 394, 
    394, 395, 395, 396, 396, 397, 398, 398, 
    399, 399, 400, 400, 401, 401, 402, 402, 
    403, 403, 404, 405, 405, 406, 406, 407, 
    407, 408, 408, 409, 410, 410, 411, 411, 
    412, 412, 413, 413, 414, 415, 415, 416, 
    416, 417, 417, 418, 419, 419, 420, 420, 
    421, 421, 422, 423, 423, 424, 424, 425, 
    425, 426, 427, 427, 428, 428, 429, 429, 
    430, 431, 431, 432, 432, 433, 434, 434, 
    435, 435, 436, 436, 437, 438, 438, 439, 
    439, 440, 441, 441, 442, 442, 443, 444, 
    444, 445, 445, 446, 447, 447, 448, 448, 
    449, 450, 450, 451, 452, 452, 453, 453, 
    454, 455, 455, 456, 456, 457, 458, 458, 
    459, 460, 460, 461, 461, 462, 463, 463, 
    464, 465, 465, 466, 466, 467, 468, 468, 
    469, 470, 470, 471, 472, 472, 473, 473, 
    474, 475, 475, 476, 477, 477, 478, 479, 
    479, 480, 481, 481, 482, 483, 483, 484, 
    485, 485, 486, 486, 487, 488, 488, 489, 
    490, 490, 491, 492, 492, 493, 494, 494, 
    495, 496, 496, 497, 498, 498, 499, 500, 
    501, 501, 502, 503, 503, 504, 505, 505, 
    506, 507, 507, 508, 509, 509, 510, 511,
};

static unsigned short two_exp_12bit_to_8bit(unsigned short raw_value)
{
    unsigned int tmp = 1 << (raw_value/512);
    tmp *= g_two_exp_fraction[raw_value % 512];
    tmp /= 256;

    return (unsigned short)(tmp & 0xffff);
}



#define LRADC_CHANNEL 4


void parameters_setup()
{
    int j;

    /* clear all the parameters at start */
    for (j=0; j<NUM_CHANNELS; j++) {
        g_overdrive_level[j] = OVERDRIVE_NORMAL_LEVEL;
        g_distortion_level[j] = 1;
        g_low_pass_level[j] = LOW_PASS_MAX_LEVEL;
        g_resonance_level[j] = 0;
        g_high_pass_level[j] = HIGH_PASS_MAX_LEVEL;
        g_volume_factor[j] = VOLUME_NORMAL_LEVEL;
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
        g_low_pass_level[j] = two_exp_12bit_to_8bit(tmp);
    }
}


void parameters_counter_increment()
{
}

