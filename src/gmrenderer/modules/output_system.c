#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "output_module.h"

#include "pilot_list.h"

#define MODULE_NAME "output_system"
struct st_output_system_uri
{
	char *uri;
	int state;
};

_pilot_list(st_output_system_uri, g_URI);

struct st_output_system_uri g_current_uri;

char *g_cmd_mime = NULL;
char *g_cmd_init = NULL;
char *g_cmd_play = NULL;
char *g_cmd_stop = NULL;
char *g_cmd_pause = NULL;
char *g_cmd_seek = NULL;

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
	else if (sscanf(line,"init=\"%[^\"]", value))
	{
		g_cmd_init = value;
	}
	else if (sscanf(line,"play=\"%[^\"]", value))
	{
		g_cmd_play = value;
	}
	else if (sscanf(line,"stop=\"%[^\"]", value))
	{
		g_cmd_stop = value;
	}
	else if (sscanf(line,"pause=\"%[^\"]", value))
	{
		g_cmd_pause = value;
	}
	return 0;
}

static int
output_system_init(void)
{
	config_read(MODULE_NAME,output_system_check);
	if ( g_cmd_init && !fork())
	{
		char *cmd = output_system_parse_cmd(g_cmd_init);
		system(cmd);
		free(cmd);
		exit(0);
	}
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
	return 0;
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

static int
output_system_play(output_transition_cb_t callback)
{
	struct st_output_system_uri *entry = pilot_list_first(g_URI);
	if (entry && !g_current_uri.uri)
	{
		if (g_cmd_play && !fork())
		{
			char *cmd = output_system_parse_cmd(g_cmd_play, entry->uri);
			system(cmd);
			free(cmd);
			exit(0);
		}
		memcpy(&g_current_uri, entry, sizeof(g_current_uri));
		pilot_list_remove(g_URI, entry);
	}
	else if (g_cmd_pause && !fork())
	{
		char *cmd = output_system_parse_cmd(g_cmd_pause);
		system(cmd);
		free(cmd);
		exit(0);
	}

	return 0;
}

static int
output_system_stop(void)
{
	if (g_cmd_stop && !fork())
	{
		char *cmd = output_system_parse_cmd(g_cmd_stop);
		system(cmd);
		free(cmd);
		exit(0);
	}
	free(g_current_uri.uri);
	g_current_uri.uri = NULL;
	return 0;
}

static int
output_system_pause(void)
{
	if (g_cmd_pause && !fork())
	{
		char *cmd = output_system_parse_cmd(g_cmd_pause);
		system(cmd);
		free(cmd);
		exit(0);
	}
	return 0;
}

static int
output_system_seek(int64_t position_nanos)
{
	if (g_cmd_seek && !fork())
	{
		char *cmd = output_system_parse_cmd(g_cmd_seek, position_nanos);
		system(cmd);
		free(cmd);
		exit(0);
	}
	return 0;
}

static int
output_system_get_position(int64_t *track_duration,
					 int64_t *track_pos)
{
	*track_pos = 0;
	return 1;
}


struct output_module system_output = {
     .shortname = "system",
	.description = "Debug framework",
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
