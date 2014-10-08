slib-y+=mupnp
mupnp_CFLAGS:=-std=gnu99 -DPILOT_LOGGING
mupnp_SOURCES:=dlnarenderer.c
mupnp_LIBRARY:= dlna dlnaprofiler_mpg123
LIBRARY+= dlna dlnaprofiler_mpg123
export LIBRARY
