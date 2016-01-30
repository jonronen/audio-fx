#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <asm/byteorder.h>


static snd_pcm_t *handle_cap, *handle_play;
static int quiet_mode = 0;
static char *audiobuf_even = NULL;
static char *audiobuf_odd = NULL;
static int even_odd_flag = 0;
static snd_pcm_uframes_t chunk_size = 0;
static unsigned period_time = 0;
static unsigned buffer_time = 0;
static snd_pcm_uframes_t period_frames = 0;
static snd_pcm_uframes_t buffer_frames = 0;
static int verbose = 0;
static size_t bits_per_sample, bits_per_frame;
static size_t chunk_bytes;
static snd_output_t *log;

static int fd = -1;

/* needed prototypes */

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define error(...) do {\
	fprintf(stderr, "%s: %d: ", __FUNCTION__, __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	putc('\n', stderr); \
} while (0)
#else
#define error(args...) do {\
	fprintf(stderr, "%s: %d: ", __FUNCTION__, __LINE__); \
	fprintf(stderr, ##args); \
	putc('\n', stderr); \
} while (0)
#endif	

static void set_params(snd_pcm_t* handle, int is_capture);
static ssize_t pcm_readv(u_char *data, size_t rcount);
static ssize_t pcm_writev(u_char *data, size_t count);

static void signal_handler(int sig)
{
	if (!quiet_mode)
		fprintf(stderr, "Aborted by signal %s...\n", strsignal(sig));
	if (handle_cap) {
		snd_pcm_close(handle_cap);
		handle_cap = NULL;
	}
	if (handle_play) {
		snd_pcm_close(handle_play);
		handle_play = NULL;
	}
	exit(EXIT_FAILURE);
}

enum {
	OPT_VERSION = 1,
	OPT_PERIOD_SIZE,
	OPT_BUFFER_SIZE
};

int main(int argc, char *argv[])
{
	int tmp, err, c;
	snd_pcm_info_t *info;

	snd_pcm_info_alloca(&info);

	err = snd_output_stdio_attach(&log, stderr, 0);
	assert(err >= 0);

	chunk_size = -1;

	err = snd_pcm_open(&handle_play, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		error("playback open error: %s", snd_strerror(err));
		return 1;
	}
	err = snd_pcm_open(&handle_cap,  "default", SND_PCM_STREAM_CAPTURE,  0);
	if (err < 0) {
		error("capture open error: %s", snd_strerror(err));
		return 1;
	}

	if ((err = snd_pcm_info(handle_cap, info)) < 0) {
		error("capture info error: %s", snd_strerror(err));
		return 1;
	}
	if ((err = snd_pcm_info(handle_play, info)) < 0) {
		error("playback info error: %s", snd_strerror(err));
		return 1;
	}

	chunk_size = 128;

	audiobuf_even = (char *)malloc(1024);
	audiobuf_odd = (char *)malloc(1024);
	if ((audiobuf_even == NULL) || (audiobuf_odd == NULL)) {
		error("not enough memory");
		return 1;
	}

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGABRT, signal_handler);
	
	set_params(handle_cap, 1);
	set_params(handle_play, 0);
	
	pcm_readv(audiobuf_even, chunk_bytes * 8 / bits_per_frame);
	while (1) {
		/* read the next buffer */
		pcm_readv(even_odd_flag ? audiobuf_even : audiobuf_odd, chunk_bytes * 8 / bits_per_frame);
		/* TODO: process before playback */
		pcm_writev(even_odd_flag ? audiobuf_odd : audiobuf_even, chunk_bytes * 8 / bits_per_sample);
		even_odd_flag = 1-even_odd_flag;
	}
	
	snd_pcm_close(handle_play);
	snd_pcm_close(handle_cap);
	free(audiobuf_even);
	free(audiobuf_odd);
	snd_output_close(log);
	snd_config_update_free_global();
	return EXIT_SUCCESS;
}


static void set_params(snd_pcm_t* handle, int is_capture)
{
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_uframes_t buffer_size;
	int err;
	size_t n;
	snd_pcm_uframes_t xfer_align;
	unsigned int rate;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		error("Broken configuration for this PCM: no configurations available");
		exit(EXIT_FAILURE);
	}
	err = snd_pcm_hw_params_set_access(handle, params,
					   SND_PCM_ACCESS_RW_NONINTERLEAVED);
	if (err < 0) {
		error("Access type not available");
		exit(EXIT_FAILURE);
	}
	err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE);
	if (err < 0) {
		error("Sample format non available");
		exit(EXIT_FAILURE);
	}
	err = snd_pcm_hw_params_set_channels(handle, params, 1);
	if (err < 0) {
		error("Channels count non available");
		exit(EXIT_FAILURE);
	}

	//rate = 44100;
	rate = 48000;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
	assert(err >= 0);
	if ((float)rate * 1.05 < 48000 || (float)rate * 0.95 > 48000) {
		if (!quiet_mode) {
			fprintf(stderr, "Warning: rate is not accurate (requested = %iHz, got = %iHz)\n", rate, 48000);
			fprintf(stderr, "         please, try the plug plugin (-Dplug:%s)\n", snd_pcm_name(handle));
		}
	}
	rate = 48000;
	if (buffer_time == 0 && buffer_frames == 0) {
		err = snd_pcm_hw_params_get_buffer_time_max(params,
							    &buffer_time, 0);
		assert(err >= 0);
		if (buffer_time > 500000)
			buffer_time = 500000;
	}
	if (period_time == 0 && period_frames == 0) {
		if (buffer_time > 0)
			period_time = buffer_time / 4;
		else
			period_frames = buffer_frames / 4;
	}
	if (period_time > 0)
		err = snd_pcm_hw_params_set_period_time_near(handle, params,
							     &period_time, 0);
	else
		err = snd_pcm_hw_params_set_period_size_near(handle, params,
							     &period_frames, 0);
	assert(err >= 0);
	if (buffer_time > 0) {
		err = snd_pcm_hw_params_set_buffer_time_near(handle, params,
							     &buffer_time, 0);
	} else {
		err = snd_pcm_hw_params_set_buffer_size_near(handle, params,
							     &buffer_frames);
	}
	assert(err >= 0);
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		error("Unable to install hw params:");
		snd_pcm_hw_params_dump(params, log);
		exit(EXIT_FAILURE);
	}
	snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	if (chunk_size == buffer_size) {
		error("Can't use period equal to buffer size (%lu == %lu)",
		      chunk_size, buffer_size);
		exit(EXIT_FAILURE);
	}
	snd_pcm_sw_params_current(handle, swparams);
	err = snd_pcm_sw_params_get_xfer_align(swparams, &xfer_align);
	if (err < 0) {
		error("Unable to obtain xfer align\n");
		exit(EXIT_FAILURE);
	}
	err = snd_pcm_sw_params_set_sleep_min(handle, swparams, 0);
	assert(err >= 0);
	n = chunk_size;
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, n);

	/* round up to closest transfer boundary */
	n = is_capture ? 1 : (buffer_size / xfer_align) * xfer_align;
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, n);
	assert(err >= 0);
	err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, buffer_size);
	assert(err >= 0);

	err = snd_pcm_sw_params_set_xfer_align(handle, swparams, xfer_align);
	assert(err >= 0);

	if (snd_pcm_sw_params(handle, swparams) < 0) {
		error("unable to install sw params:");
		snd_pcm_sw_params_dump(swparams, log);
		exit(EXIT_FAILURE);
	}

	if (verbose)
		snd_pcm_dump(handle, log);

	bits_per_sample = snd_pcm_format_physical_width(SND_PCM_FORMAT_S32_LE);
	bits_per_frame = bits_per_sample;
	chunk_bytes = chunk_size * bits_per_frame / 8;
	printf("Reallocating the audio buffers to size %d\n", chunk_bytes);
	audiobuf_even = realloc(audiobuf_even, chunk_bytes);
	audiobuf_odd = realloc(audiobuf_odd, chunk_bytes);
	if ((audiobuf_even == NULL) || (audiobuf_odd == NULL)) {
		error("not enough memory");
		exit(EXIT_FAILURE);
	}
	// fprintf(stderr, "real chunk_size = %i, frags = %i, total = %i\n", chunk_size, setup.buf.block.frags, setup.buf.block.frags * chunk_size);
}

#ifndef timersub
#define	timersub(a, b, result) \
do { \
	(result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
	(result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
	if ((result)->tv_usec < 0) { \
		--(result)->tv_sec; \
		(result)->tv_usec += 1000000; \
	} \
} while (0)
#endif

/* I/O error handler */
static void xrun(void)
{
	snd_pcm_status_t *status_cap;
	snd_pcm_status_t *status_play;
	int res;
	
	snd_pcm_status_alloca(&status_cap);
	snd_pcm_status_alloca(&status_play);
	if ((res = snd_pcm_status(handle_cap, status_cap))<0) {
		error("capture status error: %s", snd_strerror(res));
		exit(EXIT_FAILURE);
	}
	if ((res = snd_pcm_status(handle_play, status_play))<0) {
		error("playback status error: %s", snd_strerror(res));
		exit(EXIT_FAILURE);
	}
	if (snd_pcm_status_get_state(status_play) == SND_PCM_STATE_XRUN) {
		struct timeval now, diff, tstamp;
		gettimeofday(&now, 0);
		snd_pcm_status_get_trigger_tstamp(status_play, &tstamp);
		timersub(&now, &tstamp, &diff);
		fprintf(stderr, "overrun/underrun!!! (at least %.3f ms long)\n",
			diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
		if (verbose) {
			fprintf(stderr, "Status:\n");
			snd_pcm_status_dump(status_play, log);
		}
		if ((res = snd_pcm_prepare(handle_play))<0) {
			error("xrun: prepare error: %s", snd_strerror(res));
			exit(EXIT_FAILURE);
		}
		return;		/* ok, data should be accepted again */
	}
	if (snd_pcm_status_get_state(status_cap) == SND_PCM_STATE_DRAINING) {
		if (verbose) {
			fprintf(stderr, "Status(DRAINING):\n");
			snd_pcm_status_dump(status_cap, log);
		}
		if ((res = snd_pcm_prepare(handle_cap))<0) {
			error("xrun(DRAINING): prepare error: %s", snd_strerror(res));
			exit(EXIT_FAILURE);
		}
		return;
	}
	if (verbose) {
		fprintf(stderr, "Status(R/W):\n");
		snd_pcm_status_dump(status_play, log);
	}
	error("capture read/write error, state  %s", snd_pcm_state_name(snd_pcm_status_get_state(status_cap)));
	error("playback read/write error, state = %s", snd_pcm_state_name(snd_pcm_status_get_state(status_play)));
	exit(EXIT_FAILURE);
}

/* I/O suspend handler */
static void suspend(snd_pcm_t* handle)
{
	int res;

	if (!quiet_mode)
		fprintf(stderr, "Suspended. Trying resume. "); fflush(stderr);
	while ((res = snd_pcm_resume(handle)) == -EAGAIN)
		sleep(1);	/* wait until suspend flag is released */
	if (res < 0) {
		if (!quiet_mode)
			fprintf(stderr, "Failed. Restarting stream. "); fflush(stderr);
		if ((res = snd_pcm_prepare(handle)) < 0) {
			error("suspend: prepare error: %s", snd_strerror(res));
			exit(EXIT_FAILURE);
		}
	}
	if (!quiet_mode)
		fprintf(stderr, "Done.\n");
}

/* peak handler */
static void compute_max_peak(u_char *data, size_t count)
{
	signed int val, max, max_peak = 0, perc;
	static	int	run = 0;
	size_t ocount = count;
	int	format_little_endian = snd_pcm_format_little_endian(SND_PCM_FORMAT_S32_LE);	

	switch (bits_per_sample) {
	case 8: {
		signed char *valp = (signed char *)data;
		signed char mask = snd_pcm_format_silence(SND_PCM_FORMAT_S32_LE);
		while (count-- > 0) {
			val = *valp++ ^ mask;
			val = abs(val);
			if (max_peak < val)
				max_peak = val;
		}
		break;
	}
	case 16: {
		signed short *valp = (signed short *)data;
		signed short mask = snd_pcm_format_silence_16(SND_PCM_FORMAT_S32_LE);
		signed short sval;

		count /= 2;
		while (count-- > 0) {
			if (format_little_endian)
				sval = __le16_to_cpu(*valp);
			else	sval = __be16_to_cpu(*valp);
			sval = abs(sval) ^ mask;
			if (max_peak < sval)
				max_peak = sval;
			valp++;
		}
		break;
	}
	case 24: {
		unsigned char *valp = data;
		signed int mask = snd_pcm_format_silence_32(SND_PCM_FORMAT_S32_LE);

		count /= 3;
		while (count-- > 0) {
			if (format_little_endian) {
				val = valp[0] | (valp[1]<<8) | (valp[2]<<16);
			} else {
				val = (valp[0]<<16) | (valp[1]<<8) | valp[2];
			}
			/* Correct signed bit in 32-bit value */
			if (val & (1<<(bits_per_sample-1))) {
				val |= 0xff<<24;	/* Negate upper bits too */
			}
			val = abs(val) ^ mask;
			if (max_peak < val)
				max_peak = val;
			valp += 3;
		}
		break;
	}
	case 32: {
		signed int *valp = (signed int *)data;
		signed int mask = snd_pcm_format_silence_32(SND_PCM_FORMAT_S32_LE);
		count /= 4;
		while (count-- > 0) {
			if (format_little_endian)
				val = __le32_to_cpu(*valp);
			else	val = __be32_to_cpu(*valp);
			val = abs(val) ^ mask;
			if (max_peak < val)
				max_peak = val;
			valp++;
		}
		break;
	}
	default:
		if (run == 0) {
			fprintf(stderr, "Unsupported bit size %d.\n", bits_per_sample);
			run = 1;
		}
		return;
	}
	max = 1 << (bits_per_sample-1);
	if (max <= 0)
		max = 0x7fffffff;
	printf("Max peak (%li samples): 0x%08x ", (long)ocount, max_peak);
	if (bits_per_sample > 16)
		perc = max_peak / (max / 100);
	else
		perc = max_peak * 100 / max;
	for (val = 0; val < 20; val++)
		if (val <= perc / 5)
			putc('#', stdout);
		else
			putc(' ', stdout);
	printf(" %i%%\n", perc);
}

/*
 *  write function
 */

static ssize_t pcm_writev(u_char *data, size_t count)
{
	ssize_t r;
	size_t result = 0;

	if (count != chunk_size) {
		size_t offset = count;
		size_t remaining = chunk_size - count;
		snd_pcm_format_set_silence(SND_PCM_FORMAT_S32_LE, data + offset * bits_per_sample / 8, remaining);
		count = chunk_size;
	}
	while (count > 0) {
		void *bufs[1];
		size_t offset = result;
		bufs[0] = data + offset * bits_per_sample / 8;
		r = snd_pcm_writen(handle_play, bufs, count);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
			snd_pcm_wait(handle_play, 1000);
		} else if (r == -EPIPE) {
			xrun();
		} else if (r == -ESTRPIPE) {
			suspend(handle_play);
		} else if (r < 0) {
			error("writev error: %s", snd_strerror(r));
			exit(EXIT_FAILURE);
		}
		if (r > 0) {
			if (verbose > 1) {
				compute_max_peak(data, r);
			}
			result += r;
			count -= r;
		}
	}
	return result;
}

/*
 *  read function
 */

static ssize_t pcm_readv(u_char *data, size_t rcount)
{
	ssize_t r;
	size_t result = 0;
	size_t count = rcount;

	if (count != chunk_size) {
		count = chunk_size;
	}

	while (count > 0) {
		void *bufs[1];
		size_t offset = result;
		bufs[0] = data + offset * bits_per_sample / 8;
		r = snd_pcm_readn(handle_cap, bufs, count);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
			snd_pcm_wait(handle_cap, 1000);
		} else if (r == -EPIPE) {
			xrun();
		} else if (r == -ESTRPIPE) {
			suspend(handle_cap);
		} else if (r < 0) {
			error("readv error: %s", snd_strerror(r));
			exit(EXIT_FAILURE);
		}
		if (r > 0) {
			if (verbose > 1) {
				compute_max_peak(data, r);
			}
			result += r;
			count -= r;
		}
	}
	return rcount;
}
