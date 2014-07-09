V=0
ifeq ($(V),1)
quiet=
else
quiet=quiet_
endif
echo-cmd = $(if $($(quiet)cmd_$(1)), echo '  $($(quiet)cmd_$(1))';)
cmd = $(echo-cmd) $(cmd_$(1))

quiet_cmd_cc_o_c=CC $*
 cmd_cc_o_c=$(Q)$(CC) $(CFLAGS) -c -o $@ $<
quiet_cmd_ld_bin=LD $*
 cmd_ld_bin=$(Q)$(LD) $(LDFLAGS) $($*_LDFLAGS) -o $@ $^ $(LIBRARY:%=-l%) $($*_LIBRARY:%=-l%)
quiet_cmd_ld_slib=LD $*
 cmd_ld_slib=$(RM) $@ && \
	$(AR) -cvq $@ $^ > /dev/null && \
	$(RANLIB) $@
quiet_cmd_ld_dlib=LD $*
 cmd_ld_dlib=$(Q)$(LD) $(LDFLAGS) $($*_LDFLAGS) -shared -Wl,-soname,$@ -o $@ $^ $(LIBRARY:%=-l%) $($*_LIBRARY:%=-l%)

ifdef STATIC
lib-static-target:=$(addprefix $(obj)/lib,$(addsuffix $(slib-ext:%=.%),$(slib-y) $(lib-y)))
else
lib-static-target:=$(addprefix $(obj)/lib,$(addsuffix $(slib-ext:%=.%),$(slib-y)))
lib-dynamic-target:=$(addprefix $(obj)/lib,$(addsuffix $(dlib-ext:%=.%),$(lib-y)))
endif
modules-target:=$(addprefix $(obj)/,$(addsuffix $(dlib-ext:%=.%),$(modules-y)))
bin-target:=$(addprefix $(obj)/,$(addsuffix $(bin-ext:%=.%),$(bin-y)))

targets:=$(lib-static-target)
targets+=$(lib-dynamic-target)
targets+=$(bin-target)

CFLAGS+=-I./$(src) -I.
LDFLAGS+=-L./$(obj)

all: $(obj) $(targets)
	@:

$(obj):
	mkdir -p $@

$(obj)/%.o:$(src)/%.c
	@$(call cmd,cc_o_c)

$(foreach t,$(slib-y) $(lib-y) $(bin-y), $(eval $(t)-objs:=$($(t)_SOURCES:%.c=%.o)))
$(foreach t,$(slib-y) $(lib-y) $(bin-y), $(eval $(t)-objs:=$($(t)_SOURCES:%.c=%.o)))
target-objs:=$(foreach t, $(slib-y) $(lib-y) $(bin-y), $(if $($(t)-objs), $(addprefix $(obj)/$($(t)_SUBDIR)/,$($(t)-objs)), $(obj)/$($(t)_SUBDIR)/$(t).o))

.SECONDEXPANSION:
$(lib-static-target): CFLAGS+=-fPIC $($*_CFLAGS)
$(lib-static-target): $(obj)/lib%$(slib-ext:%=.%): $$(if $$(%-objs), $$(addprefix $(obj)/$$(%_SUBDIR)/,$$(%-objs)), $(obj)/$$(%_SUBDIR)/%.o)
	@$(call cmd,ld_slib)

$(lib-dynamic-target): CFLAGS+=-fPIC $($*_CFLAGS)
$(lib-dynamic-target): $(obj)/lib%$(dlib-ext:%=.%): $$(if $$(%-objs), $$(addprefix $(obj)/$$(%_SUBDIR)/,$$(%-objs)), $(obj)/$$(%_SUBDIR)/%.o)
	@$(call cmd,ld_dlib)

$(modules-target): CFLAGS+=-fPIC $($*_CFLAGS)
$(modules-target): $(obj)/$(dlib-ext:%=.%): $$(if $$(%-objs), $$(addprefix $(obj)/$$(%_SUBDIR)/,$$(%-objs)), $(obj)/$$(%_SUBDIR)/%.o)
	@$(call cmd,ld_dlib)

$(bin-target):CFLAGS+=$($*_CFLAGS)
$(bin-target): $(obj)/%$(bin-ext:%=.%): $$(if $$(%-objs), $$(addprefix $(obj)/$$(%_SUBDIR)/,$$(%-objs)), $(obj)/$$(%_SUBDIR)/%.o)
	@$(call cmd,ld_bin)

