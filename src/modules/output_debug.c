#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "upnp_connmgr.h"
#include "output_module.h"

struct st_output_debug
{
	char *current_URI;
	char *nextURI;
	int fd;
	int state;
};

enum output_state
{
	output_state_stop,
	output_state_play,
	output_state_pause,
};
static enum output_state g_state = output_state_stop;
static int
output_debug_init(void)
{
	printf("HELLO\n");
	g_state = output_state_stop;
	register_mime_type("audio/mpeg");
	register_mime_type("audio/*");
	return 0;
}

static void
output_debug_set_uri(const char *uri,
				     output_update_meta_cb_t meta_cb)
{
	printf("set %s\n", uri);
};

static void
output_debug_set_next_uri(const char *uri)
{
	printf("set %s\n", uri);
}

static int
output_debug_add_options(output_option_t *ctx)
{
	printf("option\n");
}

static int
output_debug_play(output_transition_cb_t callback)
{
	printf("play\n");
	g_state = output_state_play;
	return 0;
}

static int
output_debug_stop(void)
{
	printf("stop\n");
	g_state = output_state_stop;
	return 0;
}

static int
output_debug_pause(void)
{
	printf("pause\n");
	g_state = output_state_pause;
	return 0;
}

static int
output_debug_seek(int64_t position_nanos)
{
	printf("seek %ld\n", position_nanos);
	return 0;
}

static int
output_debug_get_position(int64_t *track_duration,
					 int64_t *track_pos)
{
	printf("position %ld\n", *track_duration);
	*track_pos = 0;
	return 1;
}

float g_volume = 0;
int output_debug_get_volume(float *v)
{
	*v = g_volume;
	printf("volume from %f\n", *v);
	return 0;
}
int output_debug_set_volume(float value)
{
	printf("volume to %f\n", value);
	g_volume = value;
	return g_volume;
}
int output_debug_get_mute(int *m)
{
	*m = 0;
	return 0;
}
int output_debug_set_mute(int m) {
	if (m)
		printf("mute\n");
	else
		printf("unmute\n");
	
	return 0;
}

struct output_module debug_output = {
     .shortname = "debug",
	.description = "Debug framework",
	.init        = output_debug_init,
	.add_options = output_debug_add_options,
	.set_uri     = output_debug_set_uri,
	.set_next_uri= output_debug_set_next_uri,
	.play        = output_debug_play,
	.stop        = output_debug_stop,
	.pause       = output_debug_pause,
	.seek        = output_debug_seek,
	.get_position = output_debug_get_position,
	.get_volume  = output_debug_get_volume,
	.set_volume  = output_debug_set_volume,
	.get_mute  = output_debug_get_mute,
	.set_mute  = output_debug_set_mute,
};

#ifdef PILOT_MODULES
#include <pilot_mods.h>
#include "config.h"

struct pilot_mods pilot_mods_info =
{
	.name = "output_debug",
	.appid = PACKAGE_APPID,
	.flags = 0,
	.type = OUTPUT,
	.version = 1,
	.api = (void *)&debug_output,
};
#endif
