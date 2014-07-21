SRCTREE=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))

CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
RANLIB=$(CROSS_COMPILE)ranlib
MAKE=make
export CC LD AR RANLIB MAKE

#CFLAGS=-g
CFLAGS+=-g -DDEBUG -DHAVE_CONFIG_H -DPILOT_MODULES -I$(SRCTREE:%=%/)include
STATIC=
export CFLAGS STATIC

DEFAULT: all

CONFIG=config
export CONFIG

include $(SRCTREE:%=%/)scripts.mk

all:
	$(Q)$(MAKE) $(build)=src/application.mk
	$(Q)$(MAKE) $(build)=src/modules.mk

	$(Q)$(MAKE) $(build)=data/conf.mk
