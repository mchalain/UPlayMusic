obj=obj
src=src
srctree=.

bin-y=
lib-y=

bin-ext=
slib-ext=a
dlib-ext=so

include ./config

#CFLAGS=-g
CFLAGS+=-g -DDEBUG -DHAVE_CONFIG_H -DPILOT_MODULES -I./include -I./src -I./src/gmrenderer
STATIC=

include $(src)/gmrenderer/mupnp.mk
include $(src)/pilot_atk/pilot_atk.mk
include $(src)/pilot_mods/pilot_mods.mk
include $(src)/application.mk

include $(src)/modules/debug.mk
include $(src)/modules/gstreamer.mk
include $(src)/modules/mpg123.mk
include $(src)/modules/system.mk
include $(src)/modules/sound_tinyalsa.mk

include ./scripts.mk

clean:
	$(RM) $(target-objs)
distclean: clean
	$(RM) $(lib-dynamic-target) $(lib-static-target)

