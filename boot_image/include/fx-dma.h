#include "dma.h"

#define PERIOD_SIZE 512
#define NUM_PERIODS 3

int g_snd_rec_buffs[NUM_PERIODS][PERIOD_SIZE] __attribute__((section(".ncbss"),nocommon));
int g_snd_play_buffs[NUM_PERIODS][PERIOD_SIZE] __attribute__((section(".ncbss"),nocommon));

struct stmp3xxx_dma_descriptor g_dma_rec_cmds[NUM_PERIODS];
struct stmp3xxx_dma_descriptor g_dma_play_cmds[NUM_PERIODS];


u32 g_snd_rec_phys[NUM_PERIODS];
u32 g_snd_play_phys[NUM_PERIODS];
int g_dma_rec_ch;
int g_dma_play_ch;

