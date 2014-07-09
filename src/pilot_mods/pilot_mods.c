#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

#include <pilot_mods.h>
#include <pilot_list.h>
#include <pilot_log.h>

struct _pilot_mods_internal
{
	char *path;
	char *name;
	void *handle;
	struct pilot_mods *module;
};
static _pilot_list(_pilot_mods_internal, g_mods); 
static char *g_mods_path = NULL;
static int g_mods_appid = -1;

static struct _pilot_mods_internal *
_pilot_mods_internal_create(char *path, char *name, void *handle, struct pilot_mods *module);
static int
_pilot_mods_load_dir(char *path, long flags, short type, short version);
static int
_pilot_mods_load(char *path, char *name, long flags, short type, short version);
static void *
_pilot_mods_load_lib(char *path, char *name);
static int
_pilot_mods_check(struct pilot_mods *mods, short type, short version);

int
pilot_mods_load(char *path, long flags, short appid, short type, short version)
{
	g_mods_appid = appid;
	return _pilot_mods_load_dir(path, flags, type, version);
}

static struct _pilot_mods_internal *
_pilot_mods_internal_create(char *path, char *name, void *handle, struct pilot_mods *module)
{
		struct _pilot_mods_internal *thiz = malloc(sizeof(*thiz));
		thiz->path = strdup(path);
		thiz->name = strdup(name);
		thiz->handle = handle;
		thiz->module = module;
		return thiz;
}

static int
_pilot_mods_load_dir(char *path, long flags, short type, short version)
{
	int ret = 0;
	DIR *dir = NULL;
	struct dirent *entry;
	dir = opendir(path);
	if (dir == NULL)
	{
		LOG_DEBUG("%s %s", path, strerror(errno));
		return -errno;
	}
	while ((entry = readdir(dir)) != NULL)
	{
		if (strstr(entry->d_name, ".so") != NULL)
		{
			_pilot_mods_load(path, entry->d_name, flags, type, version);
		}
	}
	return ret;
}

static void *
_pilot_mods_load_lib(char *path, char *name)
{
	void *handle;

	char *fullpath = NULL;

	fullpath = malloc(strlen(path) + strlen(name) + 2);
	sprintf(fullpath, "%s/%s", path, name);

	handle = dlopen(fullpath, RTLD_LAZY);
	free(fullpath);
	if (!handle)
	{
		LOG_DEBUG("error on plugin loading err : %s\n",dlerror());
		return NULL;
	}
	return handle;
}

static int
_pilot_mods_load(char *path, char *name, long flags, short type, short version)
{
	void *handle;
	struct pilot_mods *info;
	int ret = 0;

	handle = _pilot_mods_load_lib(path, name);

	info = dlsym(handle, PILOT_MODS_INFO);
	if (info != NULL)
	{
		if (_pilot_mods_check(info, type, version))
		{
			dlclose(handle);
			return -1;
		}

		if (flags & PILOT_MODS_FLAGSLAZY)
		{
			dlclose(handle);
			handle = NULL;
		}
		struct _pilot_mods_internal *mods = 
			_pilot_mods_internal_create(path, name, handle, info);
		pilot_list_append(g_mods, mods);
	}
	else
	{
		dlclose(handle);
		ret = -errno;
	}
	return ret;
}

static int
_pilot_mods_check(struct pilot_mods *module, short type, short version)
{
	int ret = 0;
	if (module->appid != g_mods_appid)
	{
		ret = -1;
	}
	if ((type != 0) && (module->type != type))
	{
		ret = -1;
	}
	if ((version != 0) &&
			(((version && 0xFF00) != (module->version && 0xFF00)) ||
			((version && 0x00FF) < (module->version && 0x00FF))))
	{
		ret = -1;
	}
	return ret;
}

struct pilot_mods *
pilot_mods_get(const char *name)
{
	struct pilot_mods *module = NULL;
	struct _pilot_mods_internal *mods = pilot_list_first(g_mods);
	while (mods && strcmp(mods->module->name, name))
	{
		mods = pilot_list_next(g_mods);
	}
	if (mods)
	{
		if (!mods->handle)
			mods->handle = _pilot_mods_load_lib(mods->path, mods->name);
		module = mods->module;
	}
	return module;
}

struct pilot_mods *
pilot_mods_first(short type, short version)
{
	struct pilot_mods *module = NULL;
	struct _pilot_mods_internal *mods = pilot_list_first(g_mods);
	while (mods && _pilot_mods_check(mods->module, type, version))
	{
		mods = pilot_list_next(g_mods);
	}
	if (mods)
	{
		if (!mods->handle)
			mods->handle = _pilot_mods_load_lib(mods->path, mods->name);
		module = mods->module;
	}
	return module;
}

struct pilot_mods *
pilot_mods_next(short type, short version)
{
	struct pilot_mods *module = NULL;
	struct _pilot_mods_internal *mods = pilot_list_next(g_mods);
	while (mods && _pilot_mods_check(mods->module, type, version))
	{
		mods = pilot_list_next(g_mods);
	}
	if (mods)
	{
		if (!mods->handle)
			mods->handle = _pilot_mods_load_lib(mods->path, mods->name);
		module = mods->module;
	}
	return module;
}
