sbin-y+=upme
upme_LIBRARY=pilot_atk mupnp $(if $(HAVE_LIBM),m)
upme_SOURCES:= \
pilot_main.c \
config.c

subdir-y+=gmrenderer/mupnp.mk
subdir-y+=pilot_atk/pilot_atk.mk
subdir-y+=pilot_mods/pilot_mods.mk
