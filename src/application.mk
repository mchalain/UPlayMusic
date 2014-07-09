bin-y+=upme
upme_CFLAGS:=
upme_LDFLAGS:=-L. -L$(obj)
upme_LIBRARY=pilot_atk pilot_mods upme_modules mupnp webserver upnp $(if $(HAVE_LIBM),m)
upme_SOURCES:= \
pilot_main.c
lib-y+=upme_modules
upme_modules_CFLAGS:=
upme_modules_LDFLAGS:=-L. -L$(obj)
upme_modules_LIBRARY=pilot_mods dl
upme_modules_SOURCES:= \
logging.c \
output_module.c \
sound_module.c \
config.c

$(foreach s, $(upme_SOURCES),$(eval $(s:%.c=%)_CFLAGS:=$(upme_CFLAGS)) )
