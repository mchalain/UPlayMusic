#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>

#include <upnp/ithread.h>

#include <mpg123.h>

#include <tinyalsa/asoundlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "upnp_connmgr.h"
#include "output_module.h"
#include "sound_module.h"

#include "pilot_list.h"
#include "pilot_log.h"
#include "pilot_mods.h"

#include "network.h"

#define MODULE_NAME "output_mpg123"
struct output_module mpg123_output;

enum e_state
{
	STOPPED,
	PLAYING,
	PAUSING,
};
enum e_state g_state = STOPPED;

struct st_output_mpg123_uri
{
	char *uri;
	enum e_state state;
	size_t position;
	struct http_info info;
};

static _pilot_list(st_output_mpg123_uri, g_URI);

static struct st_output_mpg123_uri g_current_uri;
static output_transition_cb_t g_callback;

static mpg123_pars *g_mpg123_pars;
static mpg123_handle *g_mpg123_handle = NULL;

static pthread_mutex_t g_mutex_control;
static pthread_cond_t g_cond_control;

static char *g_cmd_mime = "audio/mpeg";
static char *g_cmd_sound = NULL;
static struct sound_module *g_sound_api;

int
output_mpg123_check(char *line, int len)
{
	char *value = malloc(len + 1);
	if (sscanf(line,"mime=\"%[^\"]", value))
	{
		g_cmd_mime = value;
	}
	return 0;
}

static int
output_mpg123_init(void)
{
	int ret = -1;

	config_read(MODULE_NAME,output_mpg123_check);
	if (g_cmd_mime)
	{
		char *mime = g_cmd_mime;
		char *nextmime = mime;
		for (;*mime; mime++)
		{
			if (*mime == ',')
			{
				*mime = '\0';
				register_mime_type(nextmime);
				nextmime = mime + 1;
			}
		}
		register_mime_type(nextmime);
	}
	pthread_mutex_init(&g_mutex_control, NULL);
	pthread_cond_init(&g_cond_control, NULL);

	ret = mpg123_init();
	g_mpg123_pars = mpg123_new_pars(&ret);
	const char **decoderslist = mpg123_decoders();
	g_mpg123_handle = mpg123_new(decoderslist[0], &ret);

	if (!ret)
	{
		g_sound_api = sound_module_get();
		if (g_sound_api == NULL)
			ret = -1;
		else
		{
			mpg123_output.get_volume  = g_sound_api->get_volume;
			mpg123_output.set_volume  = g_sound_api->set_volume;
			mpg123_output.get_mute  = g_sound_api->get_mute;
			mpg123_output.set_mute  = g_sound_api->set_mute;
		}
	}
	return ret;
}

static void
output_mpg123_set_uri(const char *uri,
				     output_update_meta_cb_t meta_cb)
{
	struct st_output_mpg123_uri *entry = malloc(sizeof(*entry));
	memset(entry, 0 ,sizeof(*entry));
	entry->uri = strdup(uri);
	if (g_state == PLAYING)
		pilot_list_append(g_URI, entry);
	else
		pilot_list_insert(g_URI, entry, 0);
};

static void
output_mpg123_set_next_uri(const char *uri)
{
	struct st_output_mpg123_uri *entry = malloc(sizeof(*entry));
	memset(entry, 0 ,sizeof(*entry));
	entry->uri = strdup(uri);
	pilot_list_append(g_URI, entry);
}

static int
output_mpg123_add_options(output_option_t *ctx)
{
}

int
output_mpg123_openstream(int fdin, int *channels, int *encoding, long *rate, long *buffsize)
{
	if(mpg123_open_fd(g_mpg123_handle, fdin) != MPG123_OK)
	{
		return -1;
	}

	if (mpg123_getformat(g_mpg123_handle, rate, channels, encoding) != MPG123_OK)
	{
		return -1;
	}
	mpg123_format_none(g_mpg123_handle);
	mpg123_format(g_mpg123_handle, *rate, *channels, *encoding);

	*buffsize = mpg123_outblock(g_mpg123_handle);
	return 0;
}

void*
thread_play(void *arg)
{
	pthread_mutex_lock(&g_mutex_control);
	while (g_state != STOPPED && g_current_uri.uri)
	{
		pthread_mutex_unlock(&g_mutex_control);

		int fdin = http_get(g_current_uri.uri, &g_current_uri.info);
		if (fdin < 0)
			break;

		int  channels = 0, encoding = 0;
		long rate = 0, buffsize = 0;
		if (output_mpg123_openstream(fdin, &channels, &encoding, &rate, &buffsize))
		{
			pthread_mutex_lock(&g_mutex_control);
			break;
		}
		char *buffer;
		buffer = malloc(buffsize);

		g_sound_api->open(channels, encoding, rate);

		int err;
		do
		{
			pthread_mutex_lock(&g_mutex_control);
			while (g_state == PAUSING)
			{
				pthread_cond_wait(&g_cond_control, &g_mutex_control);
			} 
			/**
			 * stop is requested from the controler
			 **/
			if (g_state == STOPPED)
				break;
			pthread_mutex_unlock(&g_mutex_control);
			size_t done = 0;
			err = mpg123_read( g_mpg123_handle, buffer, buffsize, &done );
			g_current_uri.position += done;
			if (err == MPG123_OK)
				err = (g_sound_api->write(buffer, buffsize) == 0)? MPG123_OK : MPG123_ERR;
		} while (err == MPG123_OK);
		mpg123_close(g_mpg123_handle);
		g_sound_api->close();
		g_current_uri.position = g_current_uri.info.length;
		if (g_state == STOPPED)
			break;
		/**
		 * prepare the next stream
		 **/
		struct st_output_mpg123_uri *entry = pilot_list_next(g_URI);
		if (!entry)
		{
			(*g_callback)(PLAY_STOPPED);
			memset(&g_current_uri, 0, sizeof(g_current_uri));
			pthread_mutex_lock(&g_mutex_control);
			break;
		}
		memcpy(&g_current_uri, entry, sizeof(g_current_uri));
		g_current_uri.position = 0;
		(*g_callback)(PLAY_STARTED_NEXT_STREAM);
	}
	pthread_mutex_unlock(&g_mutex_control);
	return NULL;
}

static int
output_mpg123_play(output_transition_cb_t callback)
{
	g_callback = callback;
	g_state = PLAYING;
	if (!g_current_uri.uri)
	{
		struct st_output_mpg123_uri *entry = pilot_list_first(g_URI);
		if (entry)
		{
			memcpy(&g_current_uri, entry, sizeof(g_current_uri));
			g_current_uri.position = 0;

			pthread_t thread;
			pthread_create(&thread, NULL, thread_play, callback);
		}
	}
	pthread_cond_signal(&g_cond_control);
	return 0;
}

static int
output_mpg123_stop(void)
{
	g_state = STOPPED;
	pthread_cond_signal(&g_cond_control);
	if (g_current_uri.uri)
	{
		free(g_current_uri.uri);
		g_current_uri.uri = NULL;
	}
	return 0;
}

static int
output_mpg123_pause(void)
{
	g_state = PAUSING;
	pthread_cond_signal(&g_cond_control);
	return 0;
}

static int
output_mpg123_seek(int64_t position_nanos)
{
	return 0;
}

static int
output_mpg123_get_position(int64_t *track_duration,
					 int64_t *track_pos)
{
	*track_duration = g_current_uri.info.length;
	*track_pos = g_current_uri.position;
//	LOG_DEBUG("position %ld/%ld\n", *track_pos, *track_duration);
	return 0;
}


struct output_module mpg123_output = {
    .shortname = "mpg123",
	.description = "daemon framework",
	.init        = output_mpg123_init,
	.add_options = output_mpg123_add_options,
	.set_uri     = output_mpg123_set_uri,
	.set_next_uri= output_mpg123_set_next_uri,
	.play        = output_mpg123_play,
	.stop        = output_mpg123_stop,
	.pause       = output_mpg123_pause,
	.seek        = output_mpg123_seek,
	.get_position = output_mpg123_get_position,
	.get_volume  = NULL,
	.set_volume  = NULL,
	.get_mute  = NULL,
	.set_mute  = NULL,
};

#ifdef PILOT_MODULES
#include <pilot_mods.h>
#include "config.h"

struct pilot_mods pilot_mods_info =
{
	.name = MODULE_NAME,
	.appid = PACKAGE_APPID,
	.flags = 0,
	.type = OUTPUT,
	.version = 1,
	.api = (void *)&mpg123_output,
};
#endif
