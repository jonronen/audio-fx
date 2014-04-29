#include "system.h"
#include "audio_dma.h"
#include "serial.h"
#include "alsa/asoundlib.h"

#include <pthread.h>
#include <semaphore.h>


sem_t g_empty_cnt_sem;
sem_t g_full_cnt_sem;


static const char *device = "default";

#define NUM_BUFFERS 6
#define NUM_SAMPLES 128
int g_buffers[NUM_BUFFERS][NUM_SAMPLES];


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

static void* writer_thread(void* param)
{
    int i=0;
    snd_pcm_sframes_t frames;
    snd_pcm_t *handle_play = (snd_pcm_t*)param;

    while (1) {
        sem_wait(&g_full_cnt_sem);
        
        frames = snd_pcm_writei(
            handle_play,
            g_buffers[(i+2)%NUM_BUFFERS],
            NUM_SAMPLES
        );
        if (frames < 0)
            frames = snd_pcm_recover(handle_play, frames, 0);
        if (frames < 0) {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
            break;
        }
        if (frames > 0 && frames < NUM_SAMPLES)
            printf("Short write (expected %d, wrote %li)\n",
                NUM_SAMPLES, frames
            );

        sem_post(&g_empty_cnt_sem);
        
        i = (i+1) % NUM_BUFFERS;
    }

    return NULL;
}


void audio_dma_start()
{
    int err;
    unsigned int i, j;
    int min, max;

    snd_pcm_t *handle_cap, *handle_play;
    snd_pcm_sframes_t frames;

    pthread_t h_writer_thread;

    for (i = 0; i < NUM_BUFFERS; i++)
        for (j = 0; j < NUM_SAMPLES; j++)
            g_buffers[i][j] = 0;
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

    err = sem_init(&g_empty_cnt_sem, 0, NUM_BUFFERS);
    if (err < 0) {
        printf("sem_init error: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    err = sem_init(&g_full_cnt_sem, 0, 0);
    if (err < 0) {
        printf("sem_init error: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    err = pthread_create(
        &h_writer_thread, NULL, writer_thread, (void*)handle_play
    );
    if (err < 0) {
        printf("pthread_create error: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    while (1) {
        sem_wait(&g_empty_cnt_sem);

        frames = snd_pcm_readi(
            handle_cap,
            g_buffers[i%NUM_BUFFERS],
            NUM_SAMPLES
        );
        if (frames < 0)
            frames = snd_pcm_recover(handle_cap, frames, 0);
        if (frames < 0) {
            printf("snd_pcm_readi failed: %s\n", snd_strerror(frames));
            break;
        }
        if (frames > 0 && frames < NUM_SAMPLES)
            printf("Short read (expected %d, read %li)\n", NUM_SAMPLES, frames);

        /* TODO: insert the modifications here */

        max = -0x7fffffff;
        min = 0x7fffffff;
        for (j=0; j<NUM_SAMPLES; j++) {
            if (max < g_buffers[i%NUM_BUFFERS][j]) {
                max = g_buffers[i%NUM_BUFFERS][j];
            }
            if (min > g_buffers[i%NUM_BUFFERS][j]) {
                min = g_buffers[i%NUM_BUFFERS][j];
            }
        }
        printf("read #%d: min=%d, max=%d\n", i, min, max);

        sem_post(&g_full_cnt_sem);
        
        i = (i+1) % NUM_BUFFERS;
    }


    pthread_join(h_writer_thread, NULL);

    snd_pcm_close(handle_cap);
    snd_pcm_close(handle_play);
}

