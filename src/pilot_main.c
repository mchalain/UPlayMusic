#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "pilot_atk.h"
#include "pilot_mods.h"
#include "mupnp.h"

static struct pilot_application *g_application;
static void *g_upnp;

static const char *g_cmd_name = NULL;
static const char *g_cmd_uuid = NULL;
static const char *g_cmd_serial = NULL;
static const char *g_cmd_ip_address = NULL;
static int g_cmd_listen_port;

extern void config_read(char *section, int (*check)(char*, int));

int
upme_check(char *line, int len)
{
	int ret = 0;
	char *value = malloc(len + 1);
	if (sscanf(line,"name=\"%[^\"]", value))
	{
		g_cmd_name = value;
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

int main(int argc, const char **argv)
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

	g_upnp = upnp_start((char *)g_cmd_name, \
						(char *)g_cmd_uuid, \
						(char *)g_cmd_serial, \
						(char *)g_cmd_ip_address, \
						(int)g_cmd_listen_port);
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
