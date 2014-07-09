#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "pilot_log.h"

void
config_read(char *section, int (*check)(char*, int))
{
	FILE *file;

	file = fopen(CONFIGPATH,"r");
	if (!file)
	{
		file = fopen("./"CONFIGFILE,"r");
	}
	if (file)
	{
		char *line = NULL;
		ssize_t len;
		size_t n=0;
		len = getline(&line, &n, file);
		char *section_string = malloc(strlen(section) + 3);
		sprintf(section_string, "[%s]", section);
		while (len > 0)
		{
			if (!strncmp(line,section_string, 6))
			{
				break;
			}
			free(line);
			line = NULL;
			len = getline(&line, &n, file);
		}
		free(section_string);
		len = getline(&line, &n, file);
		while (len > 0)
		{
			if (!check(line, len))
			{
			}
			else if (!strncmp(line,"[", 1))
				break;
			free(line);
			line = NULL;
			len = getline(&line, &n, file);
		}
		if (line)
		{
			free(line);
			line = NULL;
		}
		fclose(file);
	}
	else
		LOG_DEBUG("%s", "./"CONFIGFILE);
}
