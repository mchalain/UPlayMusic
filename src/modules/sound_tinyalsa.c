#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#include <pthread.h>
#include <tinyalsa/asoundlib.h>

#include <mpg123.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef PILOT_SDK
#include "pilot_log.h"
#include "pilot_mods.h"
#endif
#include "sound_module.h"

struct fifo_s
{
        uint32_t count;
        void *buffer;
        ssize_t size;
        struct fifo_s *next;
};

struct fifo_s *
fifo_push (struct fifo_s *fifo, char *buffer, ssize_t size)
{
        struct fifo_s *entry;
        struct fifo_s *last;

        entry = fifo;
        while ((entry = entry->next) == NULL)
        {
                last = entry;
                last->count++;
        }
        entry = calloc (1, sizeof (struct fifo_s));
        entry->buffer = malloc (size);
        if (!entry->buffer)
        {
                free (entry);
                return NULL;
        }
        memcpy (entry->buffer, buffer, size);
        entry->size = size;
        if (last)
                last->next = entry;
        else
                fifo = entry;
        return fifo;
}

struct fifo_s *
fifo_pull (struct fifo_s *fifo, char **buffer, ssize_t *size)
{
        struct fifo_s *value;

        value = fifo;
        if (!value)
                return NULL;
        *buffer = malloc (value->size);
        if (!*buffer)
        {
                return NULL;
        }
        memcpy (*buffer, value->buffer, value->size);
        *size = value->size;
        fifo = fifo->next;
        free (value->buffer);
        value->buffer = NULL;
        value->size = 0;
        free (value);
        return fifo;
}

int
fifo_count (struct fifo_s *fifo)
{
        if (!fifo)
                return 0;
        return fifo->count;
}

struct sound_tinyalsa_global_s
{
        int cmd_card;
        int cmd_device;
        struct pcm *pcm;
        struct fifo_s *fifo;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        pthread_t thread;
};
struct sound_tinyalsa_global_s g = {
        .cmd_card = 0,
        .cmd_device = 0,
        .pcm = NULL,
        .fifo = NULL
};

void *
sound_tinyalsa_thread (void *arg)
{
        struct sound_tinyalsa_global_s *data = (struct sound_tinyalsa_global_s *)arg;
        char *buffer = NULL;
        ssize_t size = 0;

        while (1)
        {
                pthread_mutex_lock (&data->mutex);
                while (fifo_count(data->fifo) == 0)
                        pthread_cond_wait (&data->cond, &data->mutex);
                data->fifo = fifo_pull (data->fifo, &buffer, &size);
                pthread_mutex_unlock (&data->mutex);
                pcm_write(data->pcm, buffer, size);
        }
        return NULL;
}

int
sound_tinyalsa_check(char *line, int len)
{
	if (sscanf(line,"card=%i[^\n]", &g.cmd_card))
	{
	}
	else if (sscanf(line,"device=%i[^\n]", &g.cmd_device))
	{
	}
	return 0;
}

static int
sound_tinyalsa_open(int channels, int encoding, long rate)
{
	struct pcm_config config;
	
	unsigned int period_size = 1024;
	unsigned int period_count = 4;

	config.channels = channels;
	config.rate = rate;
	config.period_size = period_size;
	config.period_count = period_count;
	if (encoding == MPG123_ENC_SIGNED_32)
		config.format = PCM_FORMAT_S32_LE;
	else if (encoding == MPG123_ENC_SIGNED_16)
		config.format = PCM_FORMAT_S16_LE;
	config.start_threshold = 0;
	config.stop_threshold = 0;
	config.silence_threshold = 0;

	g.pcm = pcm_open(g.cmd_card, g.cmd_device, PCM_OUT, &config);

	if (g.pcm == NULL)
		return -1;
        pthread_mutex_init (&g.mutex, NULL);
        pthread_cond_init (&g.cond, NULL);
        pthread_create(&g.thread, NULL, sound_tinyalsa_thread, &g);
	return 0;
}

static int
sound_tinyalsa_write(char *buffer, ssize_t size)
{
        pthread_mutex_lock (&g.mutex);
        g.fifo = fifo_push (g.fifo, buffer, size);
        pthread_cond_signal (&g.cond);
        pthread_mutex_lock (&g.mutex);
        if (!g.fifo)
                return -1;
	return size;
}

static int
sound_tinyalsa_close(void)
{
	return pcm_close(g.pcm);
}

struct sound_module sound_tinyalsa =
{
	.open = sound_tinyalsa_open,
	.write = sound_tinyalsa_write,
	.close = sound_tinyalsa_close,
	.get_volume  = NULL,
	.set_volume  = NULL,
	.get_mute  = NULL,
	.set_mute  = NULL,
};

#ifdef PILOT_SDK
struct pilot_mods pilot_mods_info =
{
	.name = "sound_tinyalsa",
	.appid = PACKAGE_APPID,
	.flags = 0,
	.type = SOUND,
	.version = 1,
	.api = (void *)&sound_tinyalsa,
};
#endif
