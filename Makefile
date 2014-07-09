obj=obj
src=src
srctree=.

bin-y=
lib-y=

bin-ext=
slib-ext=a
dlib-ext=so

include ./config

#CFLAGS=-g
CFLAGS+=-g -DDEBUG -DHAVE_CONFIG_H -I./include -I./src/grenderer
STATIC=

include $(src)/grenderer/mupnp.mk

include ./scripts.mk

clean:
	$(RM) $(target-objs)
distclean: clean
	$(RM) $(lib-dynamic-target) $(lib-static-target)

