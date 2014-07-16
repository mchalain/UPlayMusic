OBJTREE:=obj
export OBJTREE
srctree=.

bin-y=
lib-y=

CONFIG=./config
export CONFIG

#CFLAGS=-g
CFLAGS+=-g -DDEBUG -DHAVE_CONFIG_H -DPILOT_MODULES -I./include
export CFLAGS
STATIC=

DEFAULT: all

include ./scripts.mk

all:
	make $(build)=src/gmrenderer/mupnp.mk
	make $(build)=src/pilot_atk/pilot_atk.mk
	make $(build)=src/pilot_mods/pilot_mods.mk
	make $(build)=src/application.mk

	make $(build)=src/modules/debug.mk
	make $(build)=src/modules/gstreamer.mk
	make $(build)=src/modules/mpg123.mk
	make $(build)=src/modules/system.mk
	make $(build)=src/modules/sound_tinyalsa.mk

clean: action:=_clean
clean: all

distclean: action:=_distclean
distclean: all

