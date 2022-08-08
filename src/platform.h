#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>

bool path_is_absolute(char *path);
int absolute_path(char *dest, const char *relpath);
int get_current_dir(char *dest);
int list_files(char *path, filetype_t extension_mask, fileinfo_t **info);

#endif
