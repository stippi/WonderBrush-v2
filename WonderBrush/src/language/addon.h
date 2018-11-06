// addon.h

#ifndef ADDON_H
#define ADDON_H

#include <OS.h>

typedef void (*init_strings)(const char* strings[]);

#define DEFAULT_NAME		"<language>"

struct language_addon
{
	char			path[B_PATH_NAME_LENGTH];
	char 			name[B_OS_NAME_LENGTH];
	image_id		image;
	init_strings	function;
};

language_addon*		build_addon(const char* path);
language_addon*		load_addon(const char* path);

void				destroy_addon(language_addon* addon);

status_t			save_addon(language_addon* addon,
							   const char* directory);

#define CLEANING_FREQUENCY			10
void				clean_temporary_files();

#endif // ADDON_H
