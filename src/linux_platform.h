#ifndef LINUX_PLATFORM_H
#define LINUX_PLATFORM_H

#include <dirent.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>

#include "types.h"
#include "common.h"

bool path_is_absolute(char *path)
{
    return path[0] == '/';
}


int absolute_path(char *dest, const char *relpath)
{
    char *p = realpath(relpath, dest);
    if (!p)
        return errno;
    else
        return 0;
}

int get_current_dir(char *destination)
{
    char *p = getcwd(destination, PATH_MAX+1);
    if (!p)
        return errno;
    else
        return 0;
}

/* Fills a fileinfo_t array with file properties. Also takes care of reserving
   the memory for such array. The starting position of the array is stored at
   *info.

   The resulting array is sorted by filename.

  Return code: the number of files found, or a negative number on error
  TODO: Cache / filesystem watch the result of this function
*/
int list_files(char *path, filetype_t extension_mask, fileinfo_t **info)
{
    DIR *dir = opendir(path);
    struct dirent *ent;
    int retvalue = 0;

    if (!dir) return -1;

    while ((ent = readdir(dir)))
    {
        //fprintf(stdout, "Test %d with file %s\n", ++c, ent->d_name);
        filetype_t ftype = FTYPE_UNKNOWN;

        // Ignore the current dir symlink as it is useless for our program
        if (!strcmp(ent->d_name, ".")) continue;

        if (extension_mask & FTYPE_DIR)
        {
            if (!strcmp(ent->d_name, "..")) ftype = FTYPE_DIR;
            else
            {
                // Query filesystem to know if the file is actually a directory
                struct stat fstats;
                char fullpath[PATH_MAX+1] = {0};
                strcpy(fullpath, path);
                strcat(fullpath, "/");
                strcat(fullpath, ent->d_name);
                if (stat(fullpath, &fstats))
                    fprintf(stderr, "Could not query info about file %s\n", ent->d_name);
                if (S_ISDIR(fstats.st_mode))
                    ftype = FTYPE_DIR;
            }
        }

        // Use extension_mask to filter unwanted files
        if ((extension_mask & FTYPE_WAV) && str_ends_with(ent->d_name, ".wav")) ftype = FTYPE_WAV;
        if ((extension_mask & FTYPE_MP3) && str_ends_with(ent->d_name, ".mp3")) ftype = FTYPE_MP3;
        if ((extension_mask & FTYPE_FLAC) && str_ends_with(ent->d_name, ".flac")) ftype = FTYPE_FLAC;
        if ((extension_mask & FTYPE_VORBIS) && str_ends_with(ent->d_name, ".ogg")) ftype = FTYPE_VORBIS;


        if (!(ftype & extension_mask)) continue;

        // TODO: Fill struct completely

        ++retvalue;
        *info = realloc(*info, sizeof(fileinfo_t) * retvalue);
        fileinfo_t *fi = (*info) + retvalue - 1;
        // Zero-initialize for sanity
        *fi = (fileinfo_t){0};
        strcpy(fi->filename, ent->d_name);
        fi->type = ftype;
    }

    qsort(*info, retvalue, sizeof(fileinfo_t), cmpfilename);
    closedir(dir);
    return retvalue;
}

#endif
