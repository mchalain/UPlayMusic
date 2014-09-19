lib-y+=mupnp
mupnp_CFLAGS:=-std=gnu99 -DPILOT_LOGGING
mupnp_SOURCES:=dlnarenderer.c
mupnp_LIBRARY:= dlna mpg123_profiler
