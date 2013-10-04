#ifndef __audio_dma_h__
#define __audio_dma_h__


typedef void (*audio_dma_callback_t)(
    int out_buff[],
    int in_buff[],
    unsigned int num_samples,
    unsigned int num_channels
);


void audio_setup();
void audio_dma_init(audio_dma_callback_t p_callback);
void audio_dma_start();


#endif /* __audio_dma_h__ */

