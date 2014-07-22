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
sbin-y:=
lib-y:=
slib-y:=
modules-y:=
data-y:=

srcdir?=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
ifneq ($(CONFIG),)
include $(srcdir:%/=%)/$(CONFIG)
	# CONFIG could define LD CC or/and CFLAGS
	# CONFIG must be included before "Commands for build and link"
endif
ifneq ($(file),)
include $(srcdir:%/=%)/$(file)
src=$(patsubst %/,%,$(srcdir:%/=%)/$(dir $(file)))
obj=$(patsubst %/,%,$(CURDIR:%/=%)/$(dir $(file)))
endif

##
# default Macros for installation
##
# not set variable if not into the build step
AWK?=awk
INSTALL?=install
INSTALL_PROGRAM?=$(INSTALL)
INSTALL_DATA?=$(INSTALL) -m 644

CC?=$(CROSS_COMPILE)gcc
LD?=$(CROSS_COMPILE)gcc
AR?=$(CROSS_COMPILE)ar
RANLIB?=$(CROSS_COMPILE)ranlib

prefix?=/usr/local
prefix:=$(prefix:"%"=%)
bindir?=$(prefix)/bin
bindir:=$(bindir:"%"=%)
sbindir?=$(prefix)/sbin
sbindir:=$(sbindir:"%"=%)
libdir?=$(prefix)/lib
libdir:=$(libdir:"%"=%)
includedir?=$(prefix)/include
includedir:=$(includedir:"%"=%)
datadir?=$(prefix)/share/$(PACKAGE_NAME:"%"=%)
datadir:=$(datadir:"%"=%)
pkglibdir?=$(libdir)/$(PACKAGE_NAME:"%"=%)
pkglibdir:=$(pkglibdir:"%"=%)

ifneq ($(file),)
#CFLAGS+=$(foreach macro,$(DIRECTORIES_LIST),-D$(macro)=\"$($(macro))\")
CFLAGS+=-I$(src) -I$(CURDIR) -I.
LDFLAGS+=-L$(obj) -Wl,-rpath,$(libdir)
else
export prefix bindir sbindir libdir includedir datadir pkglibdir srcdir
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
RPATH=$(wildcard $(addsuffix /.,$(wildcard $(CURDIR:%/=%)/* $(obj)/*)))
quiet_cmd_cc_o_c=CC $*
 cmd_cc_o_c=$(CC) $(CFLAGS) $($*_CFLAGS) -c -o $@ $<
quiet_cmd_ld_bin=LD $*
 cmd_ld_bin=$(LD) $(LDFLAGS) $($*_LDFLAGS) -o $@ $^ $(addprefix -L,$(RPATH)) $(LIBRARY:%=-l%) $($*_LIBRARY:%=-l%)
quiet_cmd_ld_slib=LD $*
 cmd_ld_slib=$(RM) $@ && \
	$(AR) -cvq $@ $^ > /dev/null && \
	$(RANLIB) $@
quiet_cmd_ld_dlib=LD $*
 cmd_ld_dlib=$(LD) $(LDFLAGS) $($*_LDFLAGS) -shared $(DLIB_SONAME),$(notdir $@) -o $@ $^ $(addprefix -L,$(RPATH)) $(LIBRARY:%=-l%) $($*_LIBRARY:%=-l%)

##
# objects recipes generation
##
$(foreach t,$(slib-y) $(lib-y) $(bin-y) $(sbin-y) $(modules-y), $(eval $(t)-objs:=$($(t)_SOURCES:%.c=%.o)))
target-objs:=$(foreach t, $(slib-y) $(lib-y) $(bin-y) $(sbin-y) $(modules-y), $(if $($(t)-objs), $(addprefix $(obj)/,$($(t)-objs)), $(obj)/$(t).o))

$(foreach t,$(slib-y) $(lib-y) $(bin-y) $(sbin-y) $(modules-y),$(foreach s, $($(t)_SOURCES),$(eval $(s:%.c=%)_CFLAGS:=$($(t)_CFLAGS))))

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
bin-target:=$(addprefix $(obj)/,$(addsuffix $(bin-ext:%=.%),$(bin-y) $(sbin-y)))
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
$(subdir-target): $(srcdir:%/=%)/%:
	$(Q)$(MAKE) $(build)=$*
##
# Commands for install
##
quiet_cmd_install_data=INSTALL $*
 cmd_install_data=$(INSTALL_DATA) -D $< $(DESTDIR:%=%/)$@
quiet_cmd_install_bin=INSTALL $*
 cmd_install_bin=$(INSTALL_PROGRAM) -D $< $(DESTDIR:%=%/)$@

##
# install recipes generation
##
data-install:=$(addprefix $(datadir)/,$(data-y))
include-install:=$(addprefix $(includedir)/,$(include-y))
lib-dynamic-install:=$(addprefix $(libdir)/,$(addsuffix $(dlib-ext:%=.%),$(lib-y)))
modules-install:=$(addprefix $(pkglibdir)/,$(addsuffix $(dlib-ext:%=.%),$(modules-y)))
bin-install:=$(addprefix $(bindir)/,$(addsuffix $(bin-ext:%=.%),$(bin-y)))
sbin-install:=$(addprefix $(sbindir)/,$(addsuffix $(bin-ext:%=.%),$(sbin-y)))

##
# install rules
##
$(include-install): $(includedir)/%: $(src)/%
	@$(call cmd,install_data)
$(data-install): $(datadir)/%: $(src)/%
	@$(call cmd,install_data)
$(lib-dynamic-install): $(libdir)/lib%$(dlib-ext:%=.%): $(obj)/lib%$(dlib-ext:%=.%)
	@$(call cmd,install_bin)
$(modules-install): $(pkglibdir)/%$(dlib-ext:%=.%): $(obj)/%$(dlib-ext:%=.%)
	@$(call cmd,install_bin)
$(bin-install): $(bindir)/%$(bin-ext:%=.%): $(obj)/%$(bin-ext:%=.%)
	@$(call cmd,install_bin)
$(sbin-install): $(sbindir)/%$(bin-ext:%=.%): $(obj)/%$(bin-ext:%=.%)
	@$(call cmd,install_bin)

install:=
install+=$(bin-install)
install+=$(sbin-install)
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
build:=$(action) -f $(srcdir)/scripts.mk file
_build: $(obj)/ $(CURDIR:%/=%)/config.h $(subdir-target) $(targets)
	@:

_install: action:=_install
_install: build:=$(action) -f $(srcdir)/scripts.mk file
_install: $(install) $(subdir-target)
	@:

_clean: action:=_clean
_clean: build:=$(action) -f $(srcdir)/scripts.mk file
_clean: $(subdir-target)
	$(Q)rm -f $(target-objs)

_distclean: action:=_distclean
_distclean: build:=$(action) -f $(srcdir)/scripts.mk file
_distclean: $(subdir-target)
	$(Q)rm -f $(target-objs)
	$(Q)rm -f $(targets)
	$(Q)rm -rf $(filter-out $(src),$(obj))

clean: action:=_clean
clean: build:=$(action) -f $(srcdir)/scripts.mk file
clean: all

distclean: action:=_distclean
distclean: build:=$(action) -f $(srcdir)/scripts.mk file
distclean: all
distclean:
	$(Q)rm -f $(CURDIR:/%=%)/config.h

install: action:=_install
install: build:=$(action) -f $(srcdir)/scripts.mk file
install: all

$(CURDIR:%/=%)/$(CONFIG).h: $(srcdir:%/=%)/$(CONFIG)
	@$(call cmd,config)
