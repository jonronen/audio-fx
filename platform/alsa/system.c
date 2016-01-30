#include "system.h"
#include "audio_dma.h"
#include "serial.h"
#include "alsa/asoundlib.h"

#include <pthread.h>
#include <semaphore.h>


sem_t g_empty_cnt_sem;
sem_t g_full_cnt_sem;


static const char *device = "default";

#define NUM_CHANNELS 2
#define NUM_BUFFERS 6
#define NUM_SAMPLES 64
int g_play_buffers[NUM_BUFFERS][NUM_SAMPLES*NUM_CHANNELS];
int g_rec_buffers[NUM_BUFFERS][NUM_SAMPLES*NUM_CHANNELS];


snd_pcm_t *g_handle_rec, *g_handle_play;

pthread_t g_h_writer_thread;
pthread_t g_h_reader_thread;


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

    while (1) {
        sem_wait(&g_full_cnt_sem);

        g_callback(
            g_play_buffers[i],
            g_rec_buffers[i],
            NUM_SAMPLES,
            NUM_CHANNELS
        );
        
        frames = snd_pcm_writei(
            g_handle_play,
            g_play_buffers[i],
            NUM_SAMPLES
        );
        if (frames < 0) {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
            frames = snd_pcm_recover(g_handle_play, frames, 0);
        }
        if (frames < 0) {
            printf("snd_pcm_recover failed: %s\n", snd_strerror(frames));
            break;
        }
        if (frames > 0 && frames < NUM_SAMPLES)
            printf("Short write (expected %d, wrote %ld)\n",
                NUM_SAMPLES, frames
            );

        sem_post(&g_empty_cnt_sem);
        
        i = (i+1) % NUM_BUFFERS;
    }

    return NULL;
}


void* reader_thread(void* param)
{
    int i=0;
    snd_pcm_sframes_t frames;

    while (1) {
        sem_wait(&g_empty_cnt_sem);

        frames = snd_pcm_readi(
            g_handle_rec,
            g_rec_buffers[i],
            NUM_SAMPLES
        );
        if (frames < 0) {
            printf("snd_pcm_readi failed: %s\n", snd_strerror(frames));
            frames = snd_pcm_recover(g_handle_rec, frames, 0);
        }
        if (frames < 0) {
            printf("snd_pcm_recover failed: %s\n", snd_strerror(frames));
            break;
        }
        if ((frames > 0) && (frames < NUM_SAMPLES))
            printf("Short read (expected %d, read %ld)\n", NUM_SAMPLES, frames);

        sem_post(&g_full_cnt_sem);
        
        i = (i+1) % NUM_BUFFERS;
    }

    return NULL;
}



void audio_dma_start()
{
    int err;
    unsigned int i, j;

    for (i = 0; i < NUM_BUFFERS; i++) {
        for (j = 0; j < NUM_SAMPLES; j++) {
            g_rec_buffers[i][j] = 0;
            g_play_buffers[i][j] = 0;
        }
    }

    if ((err = snd_pcm_open(
            &g_handle_rec, device, SND_PCM_STREAM_CAPTURE, 0
        )) < 0)
    {
        printf("Capture open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_open(
            &g_handle_play, device, SND_PCM_STREAM_PLAYBACK, 0)
        ) < 0)
    {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_set_params(g_handle_rec,
                                  SND_PCM_FORMAT_S32_LE,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  NUM_CHANNELS,
                                  48000,
                                  0, /* disallow resampling */
                                  1000)) < 0) {   /* 1 msec */
        printf("Capture open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_set_params(g_handle_play,
                                  SND_PCM_FORMAT_S32_LE,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  NUM_CHANNELS,
                                  48000,
                                  0, /* disallow resampling */
                                  1000)) < 0) {   /* 1 msec */
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
        &g_h_reader_thread, NULL, reader_thread, NULL
    );
    if (err < 0) {
        printf("pthread_create error: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    err = pthread_create(
        &g_h_writer_thread, NULL, writer_thread, NULL
    );
    if (err < 0) {
        printf("pthread_create error: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
}

