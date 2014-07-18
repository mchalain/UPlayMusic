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

##
# make file with targets definition
##
bin-y:=
lib-y:=
slib-y:=
modules-y:=
data-y:=

ifneq ($(CONFIG),)
include $(CONFIG)
	# CONFIG could define LD CC or/and CFLAGS
	# CONFIG must be included before "Commands for build and link"
endif
ifneq ($(file),)
include $(SRCTREE)/$(file)
src=$(SRCTREE:%=%/)$(dir $(file))
obj=$(OBJTREE:%=%/)$(dir $(file))
endif

##
# default Macros for installation
##
CC?=$(CROSS_COMPILE)gcc
LD?=$(CROSS_COMPILE)gcc
AR?=$(CROSS_COMPILE)ar
RANLIB?=$(CROSS_COMPILE)ranlib
AWK?=awk
INSTALL?=install
PREFIX?=/usr/local
LIBDIR?=$(PREFIX)/lib
BINDIR?=$(PREFIX)/bin
DATADIR=$(PREFIX)/share/$(PACKAGE_NAME:"%"=%)
PKGLIBDIR?=$(PREFIX)/lib/$(PACKAGE_NAME:"%"=%)

CFLAGS+=$(foreach macro,PREFIX LIBDIR BINDIR DATADIR PKGLIBDIR,-D$(macro)=\"$($(macro))\")

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

targets:=$(lib-static-target)
targets+=$(modules-target)
targets+=$(lib-dynamic-target)
targets+=$(bin-target)

##
# build rules
##
CFLAGS+=-I./$(src) -I.
LDFLAGS+=-L./$(obj)

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

##
# Commands for install
##
quiet_cmd_install_data=INSTALL $*
 cmd_install_data=$(INSTALL) -D $< $(DESTDIR:%=%/)$@
quiet_cmd_install_modules=INSTALL $*
 cmd_install_modules=$(INSTALL) -D $< $(DESTDIR:%=%/)$@
quiet_cmd_install_dlib=INSTALL $*
 cmd_install_dlib=$(INSTALL) -D $< $(DESTDIR:%=%/)$@
quiet_cmd_install_bin=INSTALL $*
 cmd_install_bin=$(INSTALL) -D $< $(DESTDIR:%=%/)$@

##
# install recipes generation
##
data-install:=$(addprefix $(DATADIR)/,$(data-y))
lib-dynamic-install:=$(addprefix $(LIBDIR)/,$(addsuffix $(dlib-ext:%=.%),$(lib-y)))
modules-install:=$(addprefix $(PKGLIBDIR)/,$(addsuffix $(dlib-ext:%=.%),$(modules-y)))
bin-install:=$(addprefix $(BINDIR)/,$(addsuffix $(bin-ext:%=.%),$(bin-y)))

##
# install rules
##
$(data-install): $(DATADIR)/%: $(src)/%
	@$(call cmd,install_data)
$(lib-dynamic-install): $(LIBDIR)/lib%$(dlib-ext:%=.%): $(obj)/lib%$(dlib-ext:%=.%)
	@$(call cmd,install_dlib)
$(modules-install): $(PKGLIBDIR)/%$(dlib-ext:%=.%): $(obj)/%$(dlib-ext:%=.%)
	@$(call cmd,install_modules)
$(bin-install): $(BINDIR)/%$(bin-ext:%=.%): $(obj)/%$(bin-ext:%=.%)
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
build=$(if $(action),$(action),_build) -f $(SRCTREE)/scripts.mk file

_build: config.h $(obj)/ $(targets)
	@:

_install: $(install)
	@:

_clean:
	$(Q)rm -rf $(target-objs)

_distclean: _clean
	$(Q)rm -rf $(targets)

clean: action:=_clean
clean: all

distclean: action:=_distclean
distclean: all

install: action:=_install
install: all

config.h: $(SRCTREE)/config
	@$(call cmd,config)
	
