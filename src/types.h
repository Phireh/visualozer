#ifndef TYPES_H
#define TYPES_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __linux__
#include <linux/limits.h>
#endif

typedef struct {
    char *filename;
    bool print_help;
} cli_args_t;

typedef enum {
    FTYPE_UNKNOWN = (1 << 0),
    FTYPE_DIR     = (1 << 1),
    FTYPE_WAV     = (1 << 2),
    FTYPE_MP3     = (1 << 3),
    FTYPE_FLAC    = (1 << 4),
    FTYPE_VORBIS  = (1 << 5),
} filetype_t;

typedef struct {
    uint64_t samplerate;
    uint64_t samples;
    uint64_t bytes_per_sample;
    uint64_t filesize;
    filetype_t type;
    char filename[PATH_MAX+1];
    char author[128];
    char album[128];
} fileinfo_t;

#endif
