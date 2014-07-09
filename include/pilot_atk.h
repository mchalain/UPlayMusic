#ifndef __PILOT_ATK_H__
#define __PILOT_ATK_H__

#include <stdint.h>
#include <sys/select.h>
#include <pilot_types.h>
#include <pilot_log.h>
#include <pilot_signal.h>
#include <pilot_list.h>

#define PILOT_CREATE_THIZ(type) \
	struct type *thiz; \
	thiz = calloc(1, sizeof(*thiz)); \
	if (!thiz) \
		return NULL; \
	memset(thiz, 0, sizeof(*thiz)); \

typedef void pilot_object_t;

struct pilot_option
{
	const char *name;
	const char *value;
};

struct pilot_connector
{
	struct pilot_application *application;
	int fd;
	pilot_bool_t distribut;
	_pilot_signal(pilot_connector, prepare_wait, struct pilot_connector *);
	_pilot_signal(pilot_connector, dispatch_events, struct pilot_connector *);
	_pilot_signal(pilot_connector, disconnect, struct pilot_connector *);
};

struct pilot_connector *
pilot_connector_create(struct pilot_application *application);
void
pilot_connector_destroy(struct pilot_connector *thiz);
int
pilot_connector_wait(struct pilot_connector *thiz);

struct pilot_application
{
	_pilot_list(pilot_connector, connectors);
	_pilot_list(pilot_option, options);
	fd_set rfds;
	int maxfd;
	int signal_pipe[2];
	pilot_bool_t running:1;
	pilot_bool_t dispatch:1;
};

struct pilot_application *
pilot_application_create(int argc, const char **argv);
void
pilot_application_destroy(struct pilot_application *application);
int
pilot_application_addconnector(struct pilot_application *application,
						struct pilot_connector *connector);
int
pilot_application_removeconnector(struct pilot_application *application,
						struct pilot_connector *connector);
int
pilot_application_check(struct pilot_application *application);
int
pilot_application_dispatchevents(struct pilot_application *application);
int
pilot_application_run(struct pilot_application *application);
int
pilot_application_exit(struct pilot_application *application, int ret);
const char *
pilot_application_getopt_string(struct pilot_application *application, char *name);
int
pilot_application_getopt_int(struct pilot_application *application, char *name);

struct pilot_timer
{
	struct pilot_connector *connector;
	_pilot_signal(pilot_timer, ring, int id);
};

struct pilot_timer *
pilot_timer_create(struct pilot_application *application);
void
pilot_timer_destroy(struct pilot_timer *timer);

#endif
