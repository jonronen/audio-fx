#ifndef __METRONOME_H__
#define __METRONOME_H__


#define BPMS_PER_SECOND 60
#define TICK_FREQUENCY 44100


#define MAX_DIVISION_FACTOR 16
#define MAX_PATTERN_UNITS 16


typedef enum _metronome_op_t {
    CONST_NONE,
    CONST_FULL,
    LINEAR_RISE,
    LINEAR_FALL,
    SINE_RISE,
    SINE_FALL,
    EXP_RISE,
    EXP_FALL,
    METRONOME_OP_MAX
} metronome_op_t;


void set_frequency(unsigned short freq);
void set_ops(metronome_op_t ops[], unsigned char num_ops);
void tick();


#endif /* __METRONOME_H__ */

