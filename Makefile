SRCTREE=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))

CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
RANLIB=$(CROSS_COMPILE)ranlib
export CC LD AR RANLIB

#CFLAGS=-g
CFLAGS+=-g -DDEBUG -DHAVE_CONFIG_H -DPILOT_MODULES -I$(SRCTREE:%=%/)include
STATIC=
export CFLAGS STATIC

DEFAULT: all

CONFIG=config
export CONFIG

include $(SRCTREE:%=%/)scripts.mk

all:
	$(Q)make $(build)=src/gmrenderer/mupnp.mk
	$(Q)make $(build)=src/pilot_atk/pilot_atk.mk
	$(Q)make $(build)=src/pilot_mods/pilot_mods.mk
	$(Q)make $(build)=src/application.mk

	$(Q)make $(build)=src/modules/debug.mk
	$(Q)make $(build)=src/modules/gstreamer.mk
	$(Q)make $(build)=src/modules/mpg123.mk
	$(Q)make $(build)=src/modules/system.mk
	$(Q)make $(build)=src/modules/sound_tinyalsa.mk

	$(Q)make $(build)=data/conf.mk
