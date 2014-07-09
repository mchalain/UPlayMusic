lib-$(CONFIG_SOUND_TINYALSA)+=sound_tinyalsa
sound_tinyalsa_SUBDIR:=modules
sound_tinyalsa_CFLAGS:=-std=gnu99
sound_tinyalsa_LIBRARY:=tinyalsa
sound_tinyalsa_SOURCES:= sound_tinyalsa.c

$(foreach s, $(sound_tinyalsa_SOURCES),$(eval $(sound_tinyalsa_SUBDIR)/$(s:%.c=%)_CFLAGS:=$(sound_tinyalsa_CFLAGS)) )
