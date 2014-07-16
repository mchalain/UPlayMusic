#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <upnp/ithread.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "output_module.h"

#include "pilot_list.h"
#include "pilot_log.h"

#include "network.h"

enum e_state
{
	STOPPED,
	PLAYING,
	PAUSING,
};
enum e_state g_state = STOPPED;

struct st_output_system_uri
{
	char *uri;
	enum e_state state;
	unsigned int position;
	struct http_info info;
};

static _pilot_list(st_output_system_uri, g_URI);

static struct st_output_system_uri g_current_uri;
static int g_fdout = 0;
static pthread_mutex_t g_mutex_control;
static pthread_cond_t g_cond_control;

char *g_cmd_mime = NULL;
char *g_cmd_out = NULL;

static char *
output_system_parse_cmd(char *cmd, ...)
{
	char *cmdline = NULL;
	char *it;
	int len = strlen(cmd);
	va_list ap;
	va_start(ap, cmd);

	cmdline = malloc(len + 1);
	it = cmdline;
	*it= '\0';
	enum { DATA, CMD } state = DATA;
	while(*cmd != '\0')
	{
		switch (state)
		{
			case DATA:
				if (*cmd == '%')
					state = CMD;
				else
				{
					*it = *cmd;
					it++;
				}
			break;
			case CMD:
				if (*cmd == 'u')
				{
					char *uri = va_arg(ap, char *);
					int urilen = strlen(uri);
					if (urilen < 256)
					{
						char *newcmdline;
						len += urilen - 2;
						newcmdline = realloc(cmdline, len + 1);
						if (newcmdline != cmdline)
						{
							it = newcmdline + (it - cmdline);
						}
						cmdline = newcmdline;
						while (*uri != '\0')
						{
							*it = *uri;
							it++; uri++;
						}
					}
				}
				if (*cmd == 'd')
				{
					int integer = va_arg(ap, int);
					*it++ = '0';
				}
				state = DATA;
			break;
		}
		cmd++;
	}
	*it = '\0';
	return cmdline;
}

int
output_system_check(char *line, int len)
{
	char *value = malloc(len + 1);
	if (sscanf(line,"mime=\"%[^\"]", value))
	{
		g_cmd_mime = value;
	}
	else if (sscanf(line,"out=\"%[^\"]", value))
	{
		g_cmd_out = value;
	}
	return 0;
}

static int
output_system_init(void)
{
	int ret = -1;
	
	config_read("output_system",output_system_check);
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
	LOG_DEBUG("%s", g_cmd_out);
	if (g_cmd_out)
	{
		g_fdout = open(g_cmd_out, O_WRONLY);
		if (g_fdout > 0)
		{
			LOG_DEBUG("open %s error %s", g_cmd_out, strerror(errno));
			ret = 0;
		}
	}
	LOG_DEBUG("%d", ret);
	return ret;
}

static void
output_system_set_uri(const char *uri,
				     output_update_meta_cb_t meta_cb)
{
	struct st_output_system_uri *entry = malloc(sizeof(*entry));
	memset(entry, 0 ,sizeof(*entry));
	entry->uri = strdup(uri);
	pilot_list_insert(g_URI, entry, 0);
};

static void
output_system_set_next_uri(const char *uri)
{
	struct st_output_system_uri *entry = malloc(sizeof(*entry));
	memset(entry, 0 ,sizeof(*entry));
	entry->uri = strdup(uri);
	pilot_list_append(g_URI, entry);
}

static int
output_system_add_options(struct output_option *ctx)
{
}

void*
thread_play(void *arg)
{
	pthread_mutex_lock(&g_mutex_control);
	while (g_fdout > 0 && g_state != STOPPED)
	{
		pthread_mutex_unlock(&g_mutex_control);

		struct st_output_system_uri *entry = pilot_list_first(g_URI);
		if (!entry)
		{
			pthread_mutex_lock(&g_mutex_control);
			break;
		}
		memcpy(&g_current_uri, entry, sizeof(g_current_uri));
		g_current_uri.position = 0;
		pilot_list_remove(g_URI, entry);
		int fdin = http_get(g_current_uri.uri, &g_current_uri.info);

		int len = 1024;
		char *frame[1024];

		do
		{
			pthread_mutex_lock(&g_mutex_control);
			while (g_state == PAUSING)
			{
				pthread_cond_wait(&g_cond_control, &g_mutex_control);
			}
			if (g_state == STOPPED)
				break;
			pthread_mutex_unlock(&g_mutex_control);
			len = read(fdin, frame, len);
			if (len > 0)
			{
				int ret;
				ret = write(g_fdout, frame, len);
				g_current_uri.position += ret;
				if (ret < len)
				{
					pthread_mutex_lock(&g_mutex_control);
					break;
				}
			}
		} while (len > 0);
	}
	pthread_mutex_unlock(&g_mutex_control);
	return NULL;
}

static int
output_system_play(output_transition_cb_t callback)
{
	g_state = PLAYING;
	if (!g_current_uri.uri)
	{
		pthread_t thread;
		pthread_create(&thread, NULL, thread_play, NULL);
	}
	pthread_cond_signal(&g_cond_control);
	return 0;
}

static int
output_system_stop(void)
{
	g_state = STOPPED;
	pthread_cond_signal(&g_cond_control);
	free(g_current_uri.uri);
	g_current_uri.uri = NULL;
	return 0;
}

static int
output_system_pause(void)
{
	g_state = PAUSING;
	pthread_cond_signal(&g_cond_control);
	return 0;
}

static int
output_system_seek(int64_t position_nanos)
{
	return 0;
}

static int
output_system_get_position(int64_t *track_duration,
					 int64_t *track_pos)
{
	*track_duration = g_current_uri.info.length;
	*track_pos = g_current_uri.position;
	return 0;
}


struct output_module system_output = {
     .shortname = "system 2",
	.description = "daemon framework",
	.init        = output_system_init,
	.add_options = output_system_add_options,
	.set_uri     = output_system_set_uri,
	.set_next_uri= output_system_set_next_uri,
	.play        = output_system_play,
	.stop        = output_system_stop,
	.pause       = output_system_pause,
	.seek        = output_system_seek,
	.get_position = output_system_get_position,
	.get_volume  = NULL,
	.set_volume  = NULL,
	.get_mute  = NULL,
	.set_mute  = NULL,
};

#include <pilot_mods.h>
#include "config.h"

struct pilot_mods pilot_mods_info =
{
	.name = "output_system",
	.appid = PACKAGE_APPID,
	.flags = 0,
	.type = OUTPUT,
	.version = 1,
	.api = (void *)&system_output,
};
