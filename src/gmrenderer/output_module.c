/* output_module.c - Output module frontend
 *
 * Copyright (C) 2014 Marc Chalain
 *
 * This file is part of uplaymusic.
 *
 * uplaymusic is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * uplaymusic is distributed in the hope that it will be useful,
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

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <glib.h>

#include "logging.h"
#include "output_module.h"
#ifdef HAVE_GST
#include "output_gstreamer.h"
#endif
#include "output.h"

static struct output_module *modules[] = {
#ifdef HAVE_GST
	&gstreamer_output,
#else
	// this will be a runtime error, but there is not much point
	// in waiting till then.
#error "No output configured. You need to ./configure --with-gstreamer"
#endif
};

void output_dump_modules(void)
{
	int count;
	
	count = sizeof(modules) / sizeof(struct output_module *);
	if (count == 0) {
		puts("  NONE!");
	} else {
		int i;
		for (i=0; i<count; i++) {
			printf("Available output: %s\t%s%s\n",
			       modules[i]->shortname,
			       modules[i]->description,
			       (i==0) ? " (default)" : "");
		}
	}
}

struct output_module *output_module_get(const char *shortname)
{
	struct output_module *output_module = NULL;
	int count;

	count = sizeof(modules) / sizeof(struct output_module *);
	if (count == 0) {
		Log_error("output", "No output module available");
		return -1;
	}
	if (shortname == NULL) {
		output_module = modules[0];
	} else {
		int i;
		for (i=0; i<count; i++) {
			if (strcmp(modules[i]->shortname, shortname)==0) {
				output_module = modules[i];
				break;
			}
		}
	}
	
	return output_module;
}

static GMainLoop *main_loop_ = NULL;
static void exit_loop_sighandler(int sig) {
	if (main_loop_) {
		// TODO(hzeller): revisit - this is not safe to do.
		g_main_loop_quit(main_loop_);
	}
}

int output_loop()
{
        /* Create a main loop that runs the default GLib main context */
        main_loop_ = g_main_loop_new(NULL, FALSE);

	signal(SIGINT, &exit_loop_sighandler);
	signal(SIGTERM, &exit_loop_sighandler);

        g_main_loop_run(main_loop_);

        return 0;
}
