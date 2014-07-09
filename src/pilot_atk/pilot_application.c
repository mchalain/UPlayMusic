#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <pilot_atk.h>

#define MAX(a,b)	((a) > (b))? (a): (b)

static int
_pilot_application_storeopt(struct pilot_application *application, const char *arg);

struct pilot_connector *
pilot_connector_create(struct pilot_application *application)
{
	struct pilot_connector *thiz;
	thiz = malloc(sizeof(*thiz));
	memset(thiz, 0, sizeof(*thiz));
	thiz->application = application;
	if (application != NULL)
		pilot_application_addconnector(application, thiz);
	return thiz;
}

void
pilot_connector_destroy(struct pilot_connector *thiz)
{
	if (thiz->application != NULL)
		pilot_application_removeconnector(thiz->application, thiz);
	free(thiz);
}

int
pilot_connector_wait(struct pilot_connector *thiz)
{
	int ret = 0;
	int maxfd = 0;
	fd_set rfds;

	if (thiz->fd >= 0)
	{
		FD_SET(thiz->fd, &rfds);
		//maxfd = MAX(maxfd, thiz->fd + 1);
		maxfd = thiz->fd + 1;
		pilot_emit(thiz, prepare_wait, thiz);
		/// after prepare_wait, an event could be alreay availlable
		if (thiz->distribut == 0)
		{
			ret = select(maxfd, &rfds, NULL, NULL, NULL);
			if (ret > 0 && FD_ISSET(thiz->fd, &rfds))
			{
				LOG_DEBUG("on %d",thiz->fd);
				thiz->distribut = 1;
			}
		}
		if (thiz->distribut) {
			pilot_emit(thiz, dispatch_events, thiz);
		}
		ret = thiz->distribut;
		thiz->distribut = 0;
	}
	else
		ret = -1;
	return ret;
}

static int
_platform_application_dispatch_events(struct pilot_application *application);

struct pilot_application *
pilot_application_create(int argc, const char **argv)
{
	struct pilot_application *application;
	application = malloc(sizeof(*application));
	memset(application, 0, sizeof(*application));

	int i;
	for (i = 1; i < argc; i++)
	{
		if (_pilot_application_storeopt(application, argv[i]))
			break;
	}
/*
	pipe(application->signal_pipe);
	struct pilot_connector *connector = pilot_connector_create(application);
	connector->fd = application->signal_pipe[0];
	pilot_connect(connector, dispatch_events, application, _platform_application_dispatch_events);
	pilot_application_addconnector(application, connector);
*/
	return application;
}

void
pilot_application_destroy(struct pilot_application *application)
{
	pilot_list_destroy(application->connectors);
	free(application);
}

int
pilot_application_addconnector(struct pilot_application *application,
						struct pilot_connector *connector)
{
	pilot_list_append(application->connectors, connector);
	return 0;
}

int
pilot_application_removeconnector(struct pilot_application *application,
						struct pilot_connector *connector)
{
	pilot_list_remove(application->connectors, connector);
	return 0;
}

static int
_platform_application_dispatch_events(struct pilot_application *application)
{
	int ret;
	int event;

	ret = read(application->signal_pipe[1], &event, sizeof(event));
	return ret;
}

static int
_pilot_application_dispatch_item_events(struct pilot_application *application,
							struct pilot_connector *connector)
{
	if (connector->distribut) {
		pilot_emit(connector, dispatch_events, connector);
	}
	connector->distribut = 0;
	return 0;
}

int
pilot_application_dispatchevents(struct pilot_application *application)
{
	pilot_list_foreach(application->connectors,
			_pilot_application_dispatch_item_events, application);
	return 0;
}

static int
_pilot_application_fill_fdset(struct pilot_application *application,
							struct pilot_connector *connector)
{
	int ret = 0;
	if (connector->fd >= 0)
	{
		FD_SET(connector->fd, &application->rfds);
		application->maxfd = MAX(application->maxfd, connector->fd + 1);
		pilot_emit(connector, prepare_wait, connector);
		/// after prepare_wait, an event could be alreay availlable
		if (ret)
		{
			application->dispatch = 1;
			LOG_DEBUG("fast event");
		}
	}
	return ret;
}

static int
_pilot_application_check_fdset(struct pilot_application *application,
							struct pilot_connector *connector)
{
	if (FD_ISSET(connector->fd, &application->rfds)) {
		LOG_DEBUG("on %d",connector->fd);
		connector->distribut = 1;
	}
	return 0;
}

int
pilot_application_check(struct pilot_application *application)
{
	int ret = 0;
	application->maxfd = 0;
	FD_ZERO(&application->rfds);
	pilot_list_foreach(application->connectors,
			_pilot_application_fill_fdset, application);
/*
	if (application->maxfd == 0)
	{
		/// no more connector available, stop the application
		application->running = 0;
		return -1;
	}
*/
	/// if an event is already availlable if useless to wait
	ret = (application->dispatch)?1:0;
	LOG_DEBUG("wait %d", ret);
	if (!ret)
		ret = select(application->maxfd, &application->rfds, NULL, NULL, NULL);

	application->dispatch = 0;
	LOG_DEBUG("continue %d", ret);
	if (ret > 0)
		pilot_list_foreach(application->connectors,
			_pilot_application_check_fdset, application);
	return ret;
}

int
pilot_application_run(struct pilot_application *application)
{
	int ret = 0;

	application->running = 1;
	while (application->running && ret != -1) {
		ret = pilot_application_check(application);
		if (ret >=0)
			ret = pilot_application_dispatchevents(application);

		if (ret == -1 && errno == EINTR) {
			ret = 0;
		}
	}
	return ret;
}

int
pilot_application_exit(struct pilot_application *application, int ret)
{
	application->running = 0;
	return ret;
}

static int
_pilot_application_storeopt(struct pilot_application *application, const char *arg)
{
	if (!strncmp("--",arg,2))
	{
		
		struct pilot_option *entry = malloc(sizeof(*entry));
		memset(entry, 0, sizeof(*entry));
		entry->name = arg + 2;
		entry->value = index(arg, '=') + 1;
		pilot_list_append(application->options, entry);
	}
	return 0;
}

const char *
pilot_application_getopt_string(struct pilot_application *application, char *name)
{
	struct pilot_option *option;
	const char *value = NULL;
	int len = strlen(name);

	option = pilot_list_first(application->options);
	while (option && strncmp(name, option->name, len))
	{
		option = pilot_list_next(application->options);
	}
	if (option)
		value = option->value;
	return value;
}

int
pilot_application_getopt_int(struct pilot_application *application, char *name)
{
	struct pilot_option *option;
	int value = 0;
	int len = strlen(name);

	option = pilot_list_first(application->options);
	while (option && strncmp(name, option->name, len))
	{
		option = pilot_list_next(application->options);
	}
	if (option && option->value)
		value = atoi(option->value);
	return value;
}
