lib-$(CONFIG_PILOT_MODS)+=pilot_mods
pilot_mods_SOURCES=pilot_mods.c
pilot_mods_SUBDIR:=pilot_mods
pilot_mods_CFLAGS=-I./include
pilot_mods_LDFLAGS=-ldl
$(foreach s, $(pilot_mods_SOURCES), $(eval $(pilot_mods_SUBDIR)/$(s:%.c=%)_CFLAGS+=$(pilot_mods_CFLAGS)) )
