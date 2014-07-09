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

#include "output_module.h"
#include "pilot_mods.h"

struct output_module *output_module_get(const char *shortname)
{
	int ret = -1;
	struct pilot_mods *mod;

	ret = pilot_mods_load(MOD_DIR, 0, PACKAGE_APPID, OUTPUT, 1);
	ret = pilot_mods_load("./obj", 0, PACKAGE_APPID, OUTPUT, 1);

	if (!ret)
	{
		mod = pilot_mods_get(shortname);
		if (mod)
			return (struct output_module *)mod->api;
	}
	return NULL;
}

void output_dump_modules(void)
{
	struct pilot_mods *mod;

	mod = pilot_mods_first(OUTPUT, 1);
	while (mod)
	{
		struct output_module *output = mod->api;
		printf("Available output: %s\t%s\n",
			   output->shortname,
			   output->description);
		mod = pilot_mods_next(OUTPUT, 1);
	}
}

