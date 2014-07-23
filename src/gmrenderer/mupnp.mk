lib-y+=mupnp
mupnp_CFLAGS:=-std=gnu99 -DPILOT_LOGGING
mupnp_SOURCES:= \
upnp.c \
upnp_control.c \
upnp_renderer.c \
upnp_connmgr.c \
upnp_device.c \
upnp_transport.c \
xmldoc.c \
xmlescape.c \
variable-container.c \
output.c \
song-meta-data.c
mupnp_LIBRARY:= upnp webserver

lib-$(CONFIG_WEBSERVER_MUPNP)+=webserver
webserver_CFLAGS:=-std=gnu99 -DPILOT_LOGGING
