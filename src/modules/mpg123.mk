lib-$(CONFIG_OUTPUT_MPG123)+=output_mpg123
output_mpg123_SUBDIR:=modules
output_mpg123_CFLAGS:=-std=gnu99
output_mpg123_LIBRARY:=mpg123 upme_modules
output_mpg123_SOURCES:= output_mpg123.c network.c

$(foreach s, $(output_mpg123_SOURCES),$(eval $(output_mpg123_SUBDIR)/$(s:%.c=%)_CFLAGS:=$(output_mpg123_CFLAGS)) )
