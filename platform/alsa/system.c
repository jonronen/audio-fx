#include "system.h"
#include "audio_dma.h"
#include "serial.h"
#include "alsa/asoundlib.h"

static const char *device = "default";

#define NUM_BUFFERS 3
#define NUM_SAMPLES 128
int buffers[NUM_BUFFERS][NUM_SAMPLES];


static audio_dma_callback_t g_callback;


void audio_dma_init(audio_dma_callback_t p_callback)
{
    g_callback = p_callback;
}


int main()
{
    return fx_main();
}

void audio_setup()
{
    return;
}

void system_init()
{
    return;
}

void audio_dma_start()
{
    int err;
    unsigned int i, j;
    int min, max;
    snd_pcm_t *handle_cap, *handle_play;
    snd_pcm_sframes_t frames;
    for (i = 0; i < NUM_BUFFERS; i++)
        for (j = 0; j < NUM_SAMPLES; j++)
            buffers[i][j] = 0;
    if ((err = snd_pcm_open(
            &handle_cap, device, SND_PCM_STREAM_CAPTURE, 0
        )) < 0)
    {
        printf("Capture open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_open(
            &handle_play, device, SND_PCM_STREAM_PLAYBACK, 0)
        ) < 0)
    {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_set_params(handle_cap,
                                  SND_PCM_FORMAT_S32_LE,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  1,
                                  48000,
                                  1,
                                  5000)) < 0) {   /* 0.5sec */
        printf("Capture open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_set_params(handle_play,
                                  SND_PCM_FORMAT_S32_LE,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  1,
                                  48000,
                                  1,
                                  5000)) < 0) {   /* 0.5sec */
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    while (1) {
        frames = snd_pcm_readi(
            handle_cap,
            buffers[i%NUM_BUFFERS],
            NUM_SAMPLES
        );
        if (frames < 0)
            frames = snd_pcm_recover(handle_cap, frames, 0);
        if (frames < 0) {
            printf("snd_pcm_readi failed: %s\n", snd_strerror(err));
            break;
        }
        if (frames > 0 && frames < NUM_SAMPLES)
            printf("Short read (expected %d, read %li)\n", NUM_SAMPLES, frames);

        /* TODO: insert the modifications here */

        max = -0x7fffffff;
        min = 0x7fffffff;
        for (j=0; j<NUM_SAMPLES; j++) {
            if (max < buffers[i%NUM_BUFFERS][j]) {
                max = buffers[i%NUM_BUFFERS][j];
            }
            if (min > buffers[i%NUM_BUFFERS][j]) {
                min = buffers[i%NUM_BUFFERS][j];
            }
        }
        printf("read #%d: min=%d, max=%d\n", i, min, max);
        
        frames = snd_pcm_writei(
            handle_play,
            buffers[(i+2)%NUM_BUFFERS],
            NUM_SAMPLES
        );
        if (frames < 0)
            frames = snd_pcm_recover(handle_play, frames, 0);
        if (frames < 0) {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(err));
            break;
        }
        if (frames > 0 && frames < NUM_SAMPLES)
            printf("Short write (expected %d, wrote %li)\n",
                NUM_SAMPLES, frames
            );

        i = (i+1) % NUM_BUFFERS;
    }
    snd_pcm_close(handle_cap);
    snd_pcm_close(handle_play);
}

