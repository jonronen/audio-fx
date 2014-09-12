#ifndef __audio_dma_h__
#define __audio_dma_h__


#ifdef __cplusplus
extern "C" {
#endif


typedef void (*audio_dma_callback_t)(
    int out_buff[],
    const int in_buff[],
    unsigned int num_samples,
    unsigned int num_channels
);


void audio_setup();
void audio_dma_init(audio_dma_callback_t p_callback);
void audio_dma_start();


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* __audio_dma_h__ */

