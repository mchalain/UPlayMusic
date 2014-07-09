#ifndef __PILOT_LOG__
#define __PILOT_LOG__
#include <stdio.h>
#ifdef DEBUG
#define LOG_DEBUG(format,...) fprintf(stderr, "%s "format"\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format,...) 
#endif
#define LOG_ERROR(format,...) fprintf(stderr, "%s "format"\n", __FUNCTION__, ##__VA_ARGS__)

#endif

