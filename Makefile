OBJTREE:=obj
export OBJTREE
srctree=.

CONFIG=./config
export CONFIG

#CFLAGS=-g
CFLAGS+=-g -DDEBUG -DHAVE_CONFIG_H -DPILOT_MODULES -I./include
STATIC=
export CFLAGS STATIC

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

	make $(build)=conf.mk
