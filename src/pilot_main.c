#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "config.h"
#include "pilot_atk.h"
#include "pilot_mods.h"
#include "mupnp.h"

#define DEFAULT_NAME "UPlayMusic"
#define DEFAULT_UUID "UPlayMusic"

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

static char *
create_udn (char *interface)
{
  int sock = -1;
  char *buf;
  unsigned char *ptr;

#if (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__))
  int mib[6];
  size_t len;
  struct if_msghdr *ifm;
  struct sockaddr_dl *sdl;
#else /* Linux */
  struct ifreq ifr;
#endif

  if (!interface)
    return NULL;

#if (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__))
  mib[0] = CTL_NET;
  mib[1] = AF_ROUTE;
  mib[2] = 0;
  mib[3] = AF_LINK;
  mib[4] = NET_RT_IFLIST;

  mib[5] = if_nametoindex (interface);
  if (mib[5] == 0)
  {
    perror ("if_nametoindex");
    return NULL;
  }

  if (sysctl (mib, 6, NULL, &len, NULL, 0) < 0)
  {
    perror ("sysctl");
    return NULL;
  }

  buf = malloc (len);
  if (sysctl (mib, 6, buf, &len, NULL, 0) < 0)
  {
    perror ("sysctl");
    return NULL;
  }

  ifm = (struct if_msghdr *) buf;
  sdl = (struct sockaddr_dl*) (ifm + 1);
  ptr = (unsigned char *) LLADDR (sdl);
#else /* Linux */
  /* determine UDN according to MAC address */
  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror ("socket");
    return NULL;
  }

  strcpy (ifr.ifr_name, interface);
  strcpy (ifr.ifr_hwaddr.sa_data, "");

  if (ioctl (sock, SIOCGIFHWADDR, &ifr) < 0)
  {
    perror ("ioctl");
    return NULL;
  }

  buf = (char *) malloc (64 * sizeof (char));
  memset (buf, 0, 64);
  ptr = (unsigned char *) ifr.ifr_hwaddr.sa_data;
#endif /* (defined(BSD) || defined(__FreeBSD__)) */

  snprintf (buf, 64, "%s-%02x%02x%02x%02x%02x%02x", DEFAULT_UUID,
            (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
            (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));

  if (sock)
    close (sock);

  return buf;
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
  if (!g_cmd_name)
    g_cmd_name = strdup (DEFAULT_NAME);
	value = pilot_application_getopt_string(g_application, "uuid");
	if (value)
		g_cmd_uuid = value;
  if (!g_cmd_uuid)
    g_cmd_uuid = create_udn (g_cmd_ip_address);
	value = pilot_application_getopt_string(g_application, "serial");
	if (value)
		g_cmd_serial = value;
	value = pilot_application_getopt_string(g_application, "ip_address");
	if (value)
		g_cmd_ip_address = value;
  if (!g_cmd_ip_address)
    g_cmd_ip_address = strdup ("eth0");
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
