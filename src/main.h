#ifndef VISUALOZER_MAIN_H
#define VISUALOZER_MAIN_H

/* STDLIB includes */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __linux__
#include <dirent.h>
#include <linux/limits.h>
#endif

/* Miniaudio includes */
#include "miniaudio_config.h"
#define MINIAUDIO_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function" // ignore spureous warnings from 3rd party libs
#include "miniaudio.h"
#pragma GCC diagnostic pop


/* Nuklear / GLFW includes */
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#endif
