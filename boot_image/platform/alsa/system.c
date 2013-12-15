/*
 *  This extra small demo sends a random samples to your speakers.
 */
#include "alsa/asoundlib.h"

static char *device = "default";

#define NUM_BUFFERS 3
#define NUM_SAMPLES 128
int buffers[NUM_BUFFERS][NUM_SAMPLES];
int main(void)
{
    int err;
    unsigned int i, j;
    int min, max;
    snd_pcm_t *handle_cap, *handle_play;
    snd_pcm_sframes_t frames;
    for (i = 0; i < NUM_BUFFERS; i++)
        for (j = 0; j < NUM_SAMPLES; j++)
            buffers[i][j] = 0;
    if ((err = snd_pcm_open(&handle_cap, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        printf("Capture open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_open(&handle_play, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
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
        frames = snd_pcm_readi(handle_cap, buffers[i%3], NUM_SAMPLES);
        if (frames < 0)
            frames = snd_pcm_recover(handle_cap, frames, 0);
        if (frames < 0) {
            printf("snd_pcm_readi failed: %s\n", snd_strerror(err));
            break;
        }
        if (frames > 0 && frames < NUM_SAMPLES)
            printf("Short read (expected %d, read %li)\n", NUM_SAMPLES, frames);

        max = -0x7fffffff;
        min = 0x7fffffff;
        for (j=0; j<NUM_SAMPLES; j++) {
            if (max < buffers[i%3][j]) max = buffers[i%3][j];
            if (min > buffers[i%3][j]) min = buffers[i%3][j];
        }
        printf("read #%d: min=%d, max=%d\n", i, min, max);
        
        frames = snd_pcm_writei(handle_play, buffers[(i+2)%3], NUM_SAMPLES);
        if (frames < 0)
            frames = snd_pcm_recover(handle_play, frames, 0);
        if (frames < 0) {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(err));
            break;
        }
        if (frames > 0 && frames < NUM_SAMPLES)
            printf("Short write (expected %d, wrote %li)\n", NUM_SAMPLES, frames);

        ++i;
    }
    snd_pcm_close(handle_cap);
    snd_pcm_close(handle_play);
    return 0;
}

