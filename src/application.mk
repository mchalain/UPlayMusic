sbin-y+=upme
upme_LIBRARY:=pilot_atk mupnp $(if $(HAVE_LIBM),m)
upme_SOURCES:= \
pilot_main.c \
config.c
ifeq ($(CONFIG_DLNARENDERER),y)
upme_LIBRARY+=dlna dlnaprofiler_mpg123
endif

subdir-$(CONFIG_GMRENDERER)+=gmrenderer/mupnp.mk
subdir-$(CONFIG_DLNARENDERER)+=dlnarenderer/mupnp.mk
subdir-y+=pilot_atk/pilot_atk.mk
subdir-y+=pilot_mods/pilot_mods.mk
