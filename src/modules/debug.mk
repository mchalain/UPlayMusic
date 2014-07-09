lib-$(CONFIG_OUTPUT_DEBUG)+=output_debug
output_debug_SUBDIR:=modules
output_debug_CFLAGS:=-std=gnu99
output_debug_SOURCES:= output_debug.c

$(foreach s, $(output_debug_SOURCES),$(eval $(output_debug_SUBDIR)/$(s:%.c=%)_CFLAGS:=$(output_debug_CFLAGS)) )
