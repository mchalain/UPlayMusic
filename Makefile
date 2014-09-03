prefix=/opt/upme
datadir=$(prefix)/data
pkglibdir=$(prefix)/extensions
srcdir=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))

CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
RANLIB=$(CROSS_COMPILE)ranlib
MAKE=make
export CC LD AR RANLIB MAKE

DEFAULT: all

CONFIG=config
export CONFIG

include $(srcdir:%=%/)scripts.mk

#CFLAGS=-g
CFLAGS+=-g -DDEBUG -DHAVE_CONFIG_H -DPILOT_MODULES -I$(srcdir:%=%/)include
STATIC=
CFLAGS+=-DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\"" -DPKGLIBDIR="\"$(pkglibdir)\""
export CFLAGS STATIC

all:
	$(Q)$(MAKE) $(build)=src/application.mk
	$(Q)$(MAKE) $(build)=src/modules.mk

	$(Q)$(MAKE) $(build)=data/conf.mk
