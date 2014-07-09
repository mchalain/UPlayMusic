lib-$(CONFIG_OUTPUT_SYSTEM)+=output_system
output_system_SUBDIR:=modules
output_system_CFLAGS:=-std=gnu99
output_system_LIBRARY:=upme_modules
output_system_SOURCES:= output_system2.c network.c

$(foreach s, $(output_system_SOURCES),$(eval $(output_system_SUBDIR)/$(s:%.c=%)_CFLAGS:=$(output_system_CFLAGS)) )
