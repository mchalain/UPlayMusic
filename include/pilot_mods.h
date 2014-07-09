#ifndef __PILOT_MODS_H__
#define __PILOT_MODS_H__

#define PILOT_MODS_INFO "pilot_mods_info"
struct pilot_mods
{
	char *name;
	short appid;
	short flags;
	short type;
	short version;
	void *api;
};

#ifndef PILOT_MODS
#define PILOT_MODS_FLAGSLAZY 0x0001

int
pilot_mods_load(char *path, long flags, short appid, short type, short version);
struct pilot_mods *
pilot_mods_get(const char *name);
struct pilot_mods *
pilot_mods_first(short type, short version);
struct pilot_mods *
pilot_mods_next(short type, short version);
#endif
#endif
