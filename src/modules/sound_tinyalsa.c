#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <tinyalsa/asoundlib.h>

#include <mpg123.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pilot_log.h"
#include "pilot_mods.h"
#include "sound_module.h"

static int g_cmd_card = 0;
static int g_cmd_device = 0;
static struct pcm *g_pcm = NULL;

void
sound_tinyalsa_readconf(char *filepath)
{
	FILE *file;

	file = fopen(filepath,"r");
	if (!file)
	{
		LOG_DEBUG("error on %s", filepath);
		file = fopen("./"CONFIGFILE,"r");
	}
	if (file)
	{
		char *line = NULL;
		ssize_t len;
		size_t n=0;
		len = getline(&line, &n, file);
		while (len > 0)
		{
			if (!strcmp(line,"[sound_tinyalsa]"))
			{
				break;
			}			
			free(line);
			line = NULL;
			len = getline(&line, &n, file);
		}
		while (len > 0)
		{
			if (sscanf(line,"card=%i[^\n]", &g_cmd_card))
			{
			}
			else if (sscanf(line,"device=%i[^\n]", &g_cmd_device))
			{
			}
			free(line);
			line = NULL;
			len = getline(&line, &n, file);
		}
		fclose(file);
	}
	else
		LOG_DEBUG("%s", "./"CONFIGFILE);
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

	g_pcm = pcm_open(g_cmd_card, g_cmd_device, PCM_OUT, &config);

	if (g_pcm == NULL)
		return -1;
	return 0;
}

static int
sound_tinyalsa_write(char *buffer, int buffsize)
{
	return pcm_write(g_pcm, buffer, buffsize);
}

static int
sound_tinyalsa_close()
{
	return pcm_close(g_pcm);
}

static struct sound_module sound_tinyalsa =
{
	.open = sound_tinyalsa_open,
	.write = sound_tinyalsa_write,
	.close = sound_tinyalsa_close,
	.get_volume  = NULL,
	.set_volume  = NULL,
	.get_mute  = NULL,
	.set_mute  = NULL,
};

struct pilot_mods pilot_mods_info =
{
	.name = "sound_tinyalsa",
	.appid = PACKAGE_APPID,
	.flags = 0,
	.type = SOUND,
	.version = 1,
	.api = (void *)&sound_tinyalsa,
};
