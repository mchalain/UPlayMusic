#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "pilot_atk.h"
#include "pilot_mods.h"
#include "sound_module.h"
#include "gmrenderer/upnp.h"

static struct pilot_application *g_application;
static struct upnp *g_upnp;

static const char *g_cmd_name = NULL;
static const char *g_cmd_output = NULL;
static const char *g_cmd_sound = NULL;
static const char *g_cmd_uuid = NULL;
static const char *g_cmd_serial = NULL;
static const char *g_cmd_ip_address = NULL;
static int g_cmd_listen_port;

int
upme_check(char *line, int len)
{
	int ret = 0;
	char *value = malloc(len + 1);
	if (sscanf(line,"name=\"%[^\"]", value))
	{
		g_cmd_name = value;
	}
	else if (sscanf(line,"output=\"%[^\"]", value))
	{
		g_cmd_output = value;
	}
	else if (sscanf(line,"sound=\"%[^\"]", value))
	{
		g_cmd_sound = value;
	}
	else if (sscanf(line,"uuid=\"%[^\"]", value))
	{
		g_cmd_uuid = value;
	}
	else if (sscanf(line,"serial=\"%[^\"]", value))
	{
		g_cmd_serial = value;
	}
	else if (sscanf(line,"ip_address=\"%[^\"]", value))
	{
		g_cmd_ip_address = value;
	}
	else if (sscanf(line,"listen_port=%i[^\n]", &g_cmd_listen_port))
	{
	}
	
	return ret;
}

int pilot_main(int argc, const char **argv)
{
	int ret;
	/**
	 * Setup
	 **/
	g_application = pilot_application_create(argc, argv);

	config_read("upme",upme_check);

	const char *value;
	int integer;
	value = pilot_application_getopt_string(g_application, "name");
	if (value)
		g_cmd_name = value;
	value = pilot_application_getopt_string(g_application, "uuid");
	if (value)
		g_cmd_uuid = value;
	value = pilot_application_getopt_string(g_application, "serial");
	if (value)
		g_cmd_serial = value;
	value = pilot_application_getopt_string(g_application, "ip_address");
	if (value)
		g_cmd_ip_address = value;
	integer = pilot_application_getopt_int(g_application, "listen_port");
	if (integer)
		g_cmd_listen_port = integer;
	value = pilot_application_getopt_string(g_application, "output");
	if (value)
		g_cmd_output = value;
	value = pilot_application_getopt_string(g_application, "sound");
	if (value)
		g_cmd_sound = value;

	sound_module_set(g_cmd_sound);

	g_upnp = upnp_start((char *)g_cmd_name, \
						(char *)g_cmd_uuid, \
						(char *)g_cmd_serial, \
						(char *)g_cmd_ip_address, \
						(int)g_cmd_listen_port, \
						(char *)g_cmd_output);
	/**
	 * MainLoop
	 **/
	ret = pilot_application_run(g_application);

	/**
	 * Cleanup
	 **/
	upnp_stop(g_upnp);
	pilot_application_destroy(g_application);
	return ret;
}
