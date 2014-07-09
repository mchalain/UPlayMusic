lib-y+=mupnp
mupnp_SUBDIR:=gmrenderer
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

#mupnp_SOURCES+=logging.c

$(foreach s, $(mupnp_SOURCES),$(eval $(mupnp_SUBDIR)/$(s:%.c=%)_CFLAGS:=$(mupnp_CFLAGS)) )

lib-$(CONFIG_WEBSERVER_MUPNP)+=webserver
webserver_SUBDIR:=mupnp
webserver_CFLAGS:=-std=gnu99 -DPILOT_LOGGING

$(foreach s, webserver,$(eval $(webserver_SUBDIR)/$(s:%.c=%)_CFLAGS:=$(webserver_CFLAGS)) )
