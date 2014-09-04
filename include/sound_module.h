#ifndef __SOUND_MODULE_H__
#define __SOUND_MODULE_H__

#define SOUND	0x0002

struct sound_module
{
	int (*open)(int channels, int encoding, long rate);
	int (*write)(char *buffer, ssize_t buffsize);
	int (*close)(void);
	int (*get_volume)(float *);
	int (*set_volume)(float);
	int (*get_mute)(int *);
	int (*set_mute)(int);
};

extern struct sound_module *
sound_module_set(const char *shortname);
extern struct sound_module *
sound_module_get();

#endif
