/* upnp.c - Generic UPnP routines
 *
 * Copyright (C) 2005-2007   Ivo Clarysse
 *
 * This file is part of GMediaRender.
 *
 * GMediaRender is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GMediaRender is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GMediaRender; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA 02110-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include <pthread.h>
#include <upnp/ithread.h>

#include "logging.h"
#include "xmldoc.h"
#include "output.h"
#include "upnp.h"
#include "upnp_control.h"
#include "upnp_device.h"
#include "upnp_renderer.h"
#include "upnp_transport.h"

static const char *param_datatype_names[] = {
        [DATATYPE_STRING] =     "string",
        [DATATYPE_BOOLEAN] =    "boolean",
        [DATATYPE_I2] =         "i2",
        [DATATYPE_I4] =         "i4",
        [DATATYPE_UI2] =        "ui2",
        [DATATYPE_UI4] =        "ui4",
        [DATATYPE_UNKNOWN] =    NULL
};

static struct xmlelement *gen_specversion(struct xmldoc *doc,
                                          int major, int minor)
{
	struct xmlelement *top;

	top=xmlelement_new(doc, "specVersion");

	add_value_element_int(doc, top, "major", major);
	add_value_element_int(doc, top, "minor", minor);

	return top;
}

static struct xmlelement *gen_scpd_action(struct xmldoc *doc,
                                          struct action *act,
                                          struct argument **arglist,
                                          const char **varnames)
{
	struct xmlelement *top;
	struct xmlelement *parent,*child;

	top=xmlelement_new(doc, "action");

	add_value_element(doc, top, "name", act->action_name);
	if (arglist) {
		struct argument *arg;
		int j;
		parent=xmlelement_new(doc, "argumentList");
		xmlelement_add_element(doc, top, parent);
		for(j=0; (arg=arglist[j]); j++) {
			child=xmlelement_new(doc, "argument");
			add_value_element(doc,child,"name", arg->name);
			add_value_element(doc,child,"direction",
					  (arg->direction == PARAM_DIR_IN)
					  ? "in" : "out");
			add_value_element(doc,child,"relatedStateVariable",
					  varnames[arg->statevar]);
			xmlelement_add_element(doc, parent,child);
		}
	}
	return top;
}

static struct xmlelement *gen_scpd_actionlist(struct xmldoc *doc,
                                              struct service *srv)
{
	struct xmlelement *top;
	struct xmlelement *child;
	int i;

	top=xmlelement_new(doc, "actionList");
	for(i=0; i<srv->command_count; i++) {
		struct action *act;
		struct argument **arglist;
		const char **varnames;
		act=&(srv->actions[i]);
		arglist=srv->action_arguments[i];
		varnames=srv->variable_names;
		if (act) {
			child=gen_scpd_action(doc, act, arglist, varnames);
			xmlelement_add_element(doc, top, child);
		}
	}
	return top;
}

static struct xmlelement *gen_scpd_statevar(struct xmldoc *doc,
					    const char *name,
					    struct var_meta *meta) {
	struct xmlelement *top,*parent;
	const char **valuelist;
	const char *default_value;
	struct param_range *range;

	valuelist = meta->allowed_values;
	range = meta->allowed_range;
	default_value = meta->default_value;

	top=xmlelement_new(doc, "stateVariable");

	xmlelement_set_attribute(doc, top, "sendEvents",(meta->sendevents==SENDEVENT_YES)?"yes":"no");
	add_value_element(doc,top,"name", name);
	add_value_element(doc,top,"dataType", param_datatype_names[meta->datatype]);

	if (valuelist) {
		const char *allowed_value;
		int i;
		parent=xmlelement_new(doc, "allowedValueList");
		xmlelement_add_element(doc, top, parent);
		for(i=0; (allowed_value=valuelist[i]); i++) {
			add_value_element(doc,parent,"allowedValue", allowed_value);
		} 
	}
	if (range) {
		parent=xmlelement_new(doc, "allowedValueRange");
		xmlelement_add_element(doc, top, parent);
		add_value_element_long(doc,parent,"minimum",range->min);
		add_value_element_long(doc,parent,"maximum",range->max);
		if (range->step != 0L) {
			add_value_element_long(doc,parent,"step",range->step);
		}
	}
	if (default_value) {
		add_value_element(doc,top,"defaultValue", default_value);
	}
	return top;
}

static struct xmlelement *gen_scpd_servicestatetable(struct xmldoc *doc, struct service *srv)
{
	struct xmlelement *top;
	struct xmlelement *child;
	int i;

	top=xmlelement_new(doc, "serviceStateTable");
	for(i=0; i<srv->variable_count; i++) {
		struct var_meta *meta = &(srv->variable_meta[i]);
		const char *name = srv->variable_names[i];
		child=gen_scpd_statevar(doc,name,meta);
		xmlelement_add_element(doc, top, child);
	}
	return top;
}

static struct xmldoc *generate_scpd(struct service *srv)
{
	struct xmldoc *doc;
	struct xmlelement *root;
	struct xmlelement *child;

	doc = xmldoc_new();

	root=xmldoc_new_topelement(doc, "scpd", "urn:schemas-upnp-org:service-1-0");
	child=gen_specversion(doc,1,0);
	xmlelement_add_element(doc, root, child);

	child=gen_scpd_actionlist(doc,srv);
	xmlelement_add_element(doc, root, child);

	child=gen_scpd_servicestatetable(doc,srv);
	xmlelement_add_element(doc, root, child);
	
	return doc;
}

struct action *find_action(struct service *event_service,
			   char *action_name)
{
	struct action *event_action;
	int actionNum = 0;
	if (event_service == NULL)
		return NULL;
	while (event_action =
	       &(event_service->actions[actionNum]),
	       event_action->action_name != NULL) {
		if (strcmp(event_action->action_name, action_name) == 0)
			return event_action;
		actionNum++;
	}
	return NULL;
}

char *upnp_get_scpd(struct service *srv)
{
	char *result = NULL;
	struct xmldoc *doc;

	doc = generate_scpd(srv);
	if (doc != NULL)
	{
       		result = xmldoc_tostring(doc);
		xmldoc_free(doc);
	}
	return result;
}

static void log_variable_change(void *userdata, int var_num,
				const char *variable_name,
				const char *old_value,
				const char *variable_value) {
	const char *category = (const char*) userdata;
	int needs_newline = variable_value[strlen(variable_value) - 1] != '\n';
	// Silly terminal codes. Set to empty strings if not needed.
	const char *var_start = Log_color_allowed() ? "\033[1m\033[34m" : "";
	const char *var_end = Log_color_allowed() ? "\033[0m" : "";
	Log_info(category, "%s%s%s: %s%s",
		 var_start, variable_name, var_end,
		 variable_value, needs_newline ? "\n" : "");
}

static void init_logging(const char *log_file) {
	if (log_file != NULL) {
		Log_init(log_file);
		Log_info("main", "%s log started %s", PACKAGE_STRING, PACKAGE_VERSION);

	} else {
		fprintf(stderr, "%s started %s.\nLogging switched off. "
			"Enable with --logfile=<filename> "
			"(e.g. --logfile=/dev/stdout for console)\n",
			PACKAGE_STRING, PACKAGE_VERSION);
	}
}

struct upnp * upnp_start(char *name, char *uuid, char *serial_number, char *ip_address, int listen_port, char *output)
{
	struct upnp *upnp;
	const char *log_file = "/tmp/mupnp.log";
	// Now we're going to start threads etc, which means we need
	// to become a daemon before that.

	upnp = malloc(sizeof(*upnp));
	memset(upnp, 0, sizeof(*upnp));
	init_logging(log_file);

	upnp->renderer = upnp_renderer_descriptor(name, uuid);
	if (upnp->renderer == NULL) {
		return NULL;
	}

	if (output_init(output) != 0) {
		Log_error("main",
			  "ERROR: Failed to initialize Output subsystem");
		return NULL;
	}

	if (listen_port != 0 &&
	    (listen_port < 49152 || listen_port > 65535)) {
		// Somewhere obscure internally in libupnp, they clamp the
		// port to be outside of the IANA range, so at least 49152.
		// Instead of surprising the user by ignoring lower port
		// numbers, complain loudly.
		Log_error("main", "Parameter error: --port needs to be in "
			  "range [49152..65535] (but was set to %d)",
			  listen_port);
		return NULL;
	}
	upnp->device = upnp_device_init(upnp->renderer, ip_address, listen_port);
	if (upnp->device == NULL) {
		Log_error("main", "ERROR: Failed to initialize UPnP device");
		return NULL;
	}

	upnp_transport_init(upnp->device);
	upnp_control_init(upnp->device);

	if (Log_info_enabled()) {
		upnp_transport_register_variable_listener(log_variable_change,
							  (void*) "transport");
		upnp_control_register_variable_listener(log_variable_change,
							(void*) "control");
	}

	return upnp;
}

void upnp_stop(struct upnp *upnp)
{
	if (upnp)
	{
		upnp_device_shutdown(upnp->device);
		free(upnp);
	}
}
