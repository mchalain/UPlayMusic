##
# debug tools
##
V=0
ifeq ($(V),1)
quiet=
Q=
else
quiet=quiet_
Q=@
endif
echo-cmd = $(if $($(quiet)cmd_$(1)), echo '  $($(quiet)cmd_$(1))';)
cmd = $(echo-cmd) $(cmd_$(1))

##
# file extention definition
bin-ext=
slib-ext=a
dlib-ext=so
makefile-ext=mk

##
# make file with targets definition
##
bin-y:=
lib-y:=
slib-y:=
modules-y:=
data-y:=

SRCTREE?=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
OBJTREE?=$(CURDIR)/
export SRCTREE OBJTREE
ifneq ($(CONFIG),)
include $(SRCTREE:%/=%)/$(CONFIG)
	# CONFIG could define LD CC or/and CFLAGS
	# CONFIG must be included before "Commands for build and link"
endif
ifneq ($(file),)
include $(SRCTREE:%/=%)/$(file)
src=$(patsubst %/,%,$(SRCTREE:%/=%)/$(dir $(file)))
obj=$(patsubst %/,%,$(OBJTREE:%/=%)/$(dir $(file)))
endif

##
# default Macros for installation
##
# not set variable if not into the build step
AWK?=awk
INSTALL?=install
INSTALL_PROGRAM?=$(INSTALL)
INSTALL_DATA?=$(INSTALL) -m 644
ifneq ($(file),)
CC?=$(CROSS_COMPILE)gcc
LD?=$(CROSS_COMPILE)gcc
AR?=$(CROSS_COMPILE)ar
RANLIB?=$(CROSS_COMPILE)ranlib

DIRECTORIES_LIST:=
#PREFIX set into CONFIG
ifneq ($(PREFIX),)
prefix:=$(PREFIX:"%"=%)
else
prefix?=/usr/local
PREFIX:=$(prefix:%="%")
endif
DIRECTORIES_LIST+=PREFIX
#LIBDIR set into CONFIG
ifneq ($(LIBDIR),)
libdir:=$(LIBDIR)
else
libdir?=$(prefix)/lib
LIBDIR:=$(libdir:%="%")
endif
DIRECTORIES_LIST+=LIBDIR
#BINDIR set into CONFIG
ifneq ($(BINDIR),)
bindir:=$(BINDIR)
else
bindir?=$(prefix)/bin
BINDIR:=$(bindir:%="%")
endif
DIRECTORIES_LIST+=BINDIR
#SBINDIR set into CONFIG
ifneq ($(SBINDIR),)
sbindir:=$(SBINDIR)
else
sbindir?=$(prefix)/sbin
SBINDIR:=$(sbindir:%="%")
endif
DIRECTORIES_LIST+=SBINDIR
#DATADIR set into CONFIG
ifneq ($(DATADIR),)
datadir:=$(DATADIR)
else
datadir?=$(prefix)/share/$(PACKAGE_NAME:"%"=%)
DATADIR:=$(datadir:%="%")
endif
DIRECTORIES_LIST+=DATADIR
#PKGLIBDIR not set into CONFIG
ifneq ($(PKGLIBDIR),)
pkglibdir:=$(PKGLIBDIR)
else
pkglibdir?=$(libdir)/$(PACKAGE_NAME:"%"=%)
PKGLIBDIR:=$(pkglibdir:%="%")
endif
DIRECTORIES_LIST+=PKGLIBDIR

CFLAGS+=$(foreach macro,$(DIRECTORIES_LIST),-D$(macro)=\"$($(macro))\")
CFLAGS+=-I./$(src) -I./$(OBJTREE) -I.
LDFLAGS+=-L./$(obj) -Wl,-rpath,$(LIBDIR:"%"=%)
endif

##
# Commands for build and link
##
_DLIB_SONAME:=-soname
ifeq ($(findstring gcc,$(LD)),gcc)
DLIB_SONAME:=-Wl,$(_DLIB_SONAME)
else
DLIB_SONAME:=$(_DLIB_SONAME)
endif
RPATH=$(wildcard $(addsuffix /.,$(wildcard $(OBJTREE:%=%/)* $(obj)/*)))
quiet_cmd_cc_o_c=CC $*
 cmd_cc_o_c=$(CC) $(CFLAGS) $($*_CFLAGS) -c -o $@ $<
quiet_cmd_ld_bin=LD $*
 cmd_ld_bin=$(LD) $(LDFLAGS) $($*_LDFLAGS) -o $@ $^ $(addprefix -L,$(RPATH)) $(LIBRARY:%=-l%) $($*_LIBRARY:%=-l%)
quiet_cmd_ld_slib=LD $*
 cmd_ld_slib=$(RM) $@ && \
	$(AR) -cvq $@ $^ > /dev/null && \
	$(RANLIB) $@
quiet_cmd_ld_dlib=LD $*
 cmd_ld_dlib=$(LD) $(LDFLAGS) $($*_LDFLAGS) -shared $(DLIB_SONAME),$@ -o $@ $^ $(addprefix -L,$(RPATH)) $(LIBRARY:%=-l%) $($*_LIBRARY:%=-l%)

##
# objects recipes generation
##
$(foreach t,$(slib-y) $(lib-y) $(bin-y), $(eval $(t)-objs:=$($(t)_SOURCES:%.c=%.o)))
$(foreach t,$(slib-y) $(lib-y) $(bin-y), $(eval $(t)-objs:=$($(t)_SOURCES:%.c=%.o)))
target-objs:=$(foreach t, $(slib-y) $(lib-y) $(bin-y) $(modules-y), $(if $($(t)-objs), $(addprefix $(obj)/,$($(t)-objs)), $(obj)/$(t).o))

$(foreach t,$(slib-y) $(lib-y) $(bin-y) $(modules-y),$(foreach s, $($(t)_SOURCES),$(eval $(s:%.c=%)_CFLAGS:=$($(t)_CFLAGS))))

##
# targets recipes generation
##
slib-y:=$(addprefix lib,$(slib-y))
lib-y:=$(addprefix lib,$(lib-y))
ifdef STATIC
lib-static-target:=$(addprefix $(obj)/,$(addsuffix $(slib-ext:%=.%),$(slib-y) $(lib-y)))
else
lib-static-target:=$(addprefix $(obj)/,$(addsuffix $(slib-ext:%=.%),$(slib-y)))
lib-dynamic-target:=$(addprefix $(obj)/,$(addsuffix $(dlib-ext:%=.%),$(lib-y)))
endif
modules-target:=$(addprefix $(obj)/,$(addsuffix $(dlib-ext:%=.%),$(modules-y)))
bin-target:=$(addprefix $(obj)/,$(addsuffix $(bin-ext:%=.%),$(bin-y)))
subdir-target:=$(wildcard $(addprefix $(src)/,$(addsuffix /Makefile,$(subdir-y))))
subdir-target+=$(wildcard $(addprefix $(src)/,$(addsuffix /*$(makefile-ext:%=.%),$(subdir-y))))
subdir-target+=$(if $(strip $(subdir-target)),,$(wildcard $(addprefix $(src)/,$(subdir-y))))

targets:=
targets+=$(lib-static-target)
targets+=$(modules-target)
targets+=$(lib-dynamic-target)
targets+=$(bin-target)

##
# build rules
##
$(obj)/%.o:$(src)/%.c
	@$(call cmd,cc_o_c)

$(obj)/:
	$(Q)mkdir -p $@

.SECONDEXPANSION:
$(lib-static-target): $(obj)/lib%$(slib-ext:%=.%): $$(if $$(%-objs), $$(addprefix $(obj)/,$$(%-objs)), $(obj)/%.o)
	@$(call cmd,ld_slib)

$(lib-dynamic-target): CFLAGS+=-fPIC
$(lib-dynamic-target): $(obj)/lib%$(dlib-ext:%=.%): $$(if $$(%-objs), $$(addprefix $(obj)/,$$(%-objs)), $(obj)/%.o)
	@$(call cmd,ld_dlib)

$(modules-target): CFLAGS+=-fPIC
$(modules-target): $(obj)/%$(dlib-ext:%=.%): $$(if $$(%-objs), $$(addprefix $(obj)/,$$(%-objs)), $(obj)/%.o)
	@$(call cmd,ld_dlib)

$(bin-target): $(obj)/%$(bin-ext:%=.%): $$(if $$(%-objs), $$(addprefix $(obj)/,$$(%-objs)), $(obj)/%.o)
	@$(call cmd,ld_bin)

.PHONY:$(subdir-target)
$(subdir-target): $(SRCTREE:%/=%)/%:
	$(Q)$(MAKE) $(build)=$*
##
# Commands for install
##
quiet_cmd_install_data=INSTALL $*
 cmd_install_data=$(INSTALL_DATA) -D $< $(DESTDIR:%=%/)$@
quiet_cmd_install_modules=INSTALL $*
 cmd_install_modules=$(INSTALL_PROGRAM) -D $< $(DESTDIR:%=%/)$@
quiet_cmd_install_dlib=INSTALL $*
 cmd_install_dlib=$(INSTALL_PROGRAM) -D $< $(DESTDIR:%=%/)$@
quiet_cmd_install_bin=INSTALL $*
 cmd_install_bin=$(INSTALL_PROGRAM) -D $< $(DESTDIR:%=%/)$@

##
# install recipes generation
##
data-install:=$(addprefix $(datadir)/,$(data-y))
lib-dynamic-install:=$(addprefix $(libdir)/,$(addsuffix $(dlib-ext:%=.%),$(lib-y)))
modules-install:=$(addprefix $(pkglibdir)/,$(addsuffix $(dlib-ext:%=.%),$(modules-y)))
bin-install:=$(addprefix $(bindir)/,$(addsuffix $(bin-ext:%=.%),$(bin-y)))

##
# install rules
##
$(data-install): $(datadir)/%: $(src)/%
	@$(call cmd,install_data)
$(lib-dynamic-install): $(libdir)/lib%$(dlib-ext:%=.%): $(obj)/lib%$(dlib-ext:%=.%)
	@$(call cmd,install_dlib)
$(modules-install): $(pkglibdir)/%$(dlib-ext:%=.%): $(obj)/%$(dlib-ext:%=.%)
	@$(call cmd,install_modules)
$(bin-install): $(bindir)/%$(bin-ext:%=.%): $(obj)/%$(bin-ext:%=.%)
	@$(call cmd,install_bin)

install:=$(bin-install)
install+=$(lib-dynamic-install)
install+=$(modules-install)
install+=$(data-install)

##
# commands for configuration
##
empty=
space=$(empty) $(empty)
quote="
sharp=\#
quiet_cmd_config=CONFIG $*
 cmd_config=$(AWK) -F= '$$1 != $(quote)$(quote) {print $(quote)$(sharp)define$(space)$(quote)$$1$(quote)$(space)$(quote)$$2}' $< > $@

##
# main entries
##
action:=_build
build:=$(action) -f $(SRCTREE)/scripts.mk file
_build: $(obj)/ $(OBJTREE:%=%/)config.h $(subdir-target) $(targets)
	@:

_install: action:=_install
_install: build:=$(action) -f $(SRCTREE)/scripts.mk file
_install: $(install) $(subdir-target)
	@:

_clean: action:=_clean
_clean: build:=$(action) -f $(SRCTREE)/scripts.mk file
_clean: $(subdir-target)
	$(Q)rm -f $(target-objs)

_distclean: action:=_distclean
_distclean: build:=$(action) -f $(SRCTREE)/scripts.mk file
_distclean: $(subdir-target)
	$(Q)rm -f $(target-objs)
	$(Q)rm -f $(targets)
	$(Q)rm -rf $(filter-out $(src),$(obj))

clean: action:=_clean
clean: build:=$(action) -f $(SRCTREE)/scripts.mk file
clean: all

distclean: action:=_distclean
distclean: build:=$(action) -f $(SRCTREE)/scripts.mk file
distclean: all
distclean:
	$(Q)rm -f $(OBJTREE:%=%/)config.h

install: action:=_install
install: build:=$(action) -f $(SRCTREE)/scripts.mk file
install: all

$(OBJTREE:%=%/)config.h: $(SRCTREE:%=%/)$(CONFIG)
	@$(call cmd,config)
