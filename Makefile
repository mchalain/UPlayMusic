OBJTREE:=obj
export OBJTREE
srctree=.

CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
RANLIB=$(CROSS_COMPILE)ranlib
export CC LD AR RANLIB

#CFLAGS=-g
CFLAGS+=-g -DDEBUG -DHAVE_CONFIG_H -DPILOT_MODULES -I./include
STATIC=
export CFLAGS STATIC

DEFAULT: all

CONFIG=./config
export CONFIG

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
