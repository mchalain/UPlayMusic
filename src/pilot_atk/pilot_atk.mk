slib-$(CONFIG_PILOT_ATK)+=pilot_atk
pilot_atk_SOURCES=pilot_application.c
pilot_atk_$(CONFIG_PILOT_TIMER)+=pilot_timer.c
pilot_atk_SOURCES+=$(pilot_atk_y)
pilot_atk_SUBDIR:=pilot_atk
$(foreach s, $(pilot_atk-objs), $(eval $(pilot_atk_SUBDIR)/$(s:%.c=%)_CFLAGS+=$(pilot_atk_CFLAGS)) )
