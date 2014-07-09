lib-$(CONFIG_OUTPUT_GSTREAMER)+=output_gstreamer
output_gstreamer_SUBDIR:=modules
output_gstreamer_CFLAGS:=-std=gnu99 -I$(shell pkg-config --cflags glib-2.0) -I$(shell pkg-config --cflags gstreamer-0.10) -Isrc/mupnp
output_gstreamer_SOURCES:= output_gstreamer.c

$(foreach s, $(output_gstreamer_SOURCES),$(eval $(output_gstreamer_SUBDIR)/$(s:%.c=%)_CFLAGS:=$(output_gstreamer_CFLAGS)) )
