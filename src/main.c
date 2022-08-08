#include "main.h"

/* Custom types */
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

/* Function signatures */
void draw_gui();
void draw_file_picker();
int get_current_dir(char *dest);
int enter_directory(char *relpath);

int list_files(char *path, filetype_t extension_mask, fileinfo_t **info);
bool str_ends_with(const char *haystack, const char *needle);

/* Global vars */

const int32_t default_width = 1024;
const int32_t default_height = 720;

static double cursor_pos_x = 0;
static double cursor_pos_y = 0;
static double delta_x = 0;
static double delta_y = 0;

int window_drag_active;
int32_t mouse_drag_initial_x;
int32_t mouse_drag_initial_y;

/* Global variables */

bool dragging_window = false;
bool closing_window = false;
GLFWwindow *win = NULL;
int width;
int height;

fileinfo_t *curr_dir_files;
size_t curr_dir_files_length;
char topdir[PATH_MAX+1] = ".";


struct nk_glfw glfw = {0};
struct nk_context *ctx;
struct nk_colorf bg;


// TODO: Make a nicer error callback for glfw
static void error_callback(int e, const char *d)
{
    fprintf(stderr, "Error %d: %s\n", e, d);
}

static void window_size_callback(GLFWwindow *window, int new_width, int new_height)
{
    glfw.width = new_width;
    glfw.height = new_height;
    printf("Resizing to %d w %d h\n", glfw.width, glfw.height);
    draw_gui();
}


// TODO: This is placeholder code that just outputs the sound out of the decoder, for now
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}

int parse_args(int argc, char **argv, cli_args_t *cli_args)
{
    for (int i = 1; i < argc; ++i)
    {
        char *p = argv[i];
        if (p[0] == '-')
        {
            if (p[1] == '-')
            {
                /* Long opts */
            }
            else
            {
                /* Short opts */
                if (!strcmp(p, "-f"))
                {
                    if (i < argc - 1)
                    {
                        cli_args->filename = calloc(strlen(argv[i+1]), 1);
                        strcpy(cli_args->filename, argv[++i]);
                    }
                    else
                        goto error_parsing;
                }
            }
        }
    }

    return 0;
error_parsing:
    return 1;
}


void print_usage()
{
    printf("Usage: visualozer -f <file>\n");
}


int main(int argc, char *argv[])
{
    // TODO: Set locale before initialization ?

    /* Argument parsing */
    cli_args_t cli_args = {};

    int errorcode = parse_args(argc, argv, &cli_args);

    if (errorcode           || // parsing failed
        cli_args.print_help || // just print help and exit
        !cli_args.filename)    // required args missing
    {
        print_usage();
        return errorcode;
    }

    if (get_current_dir(topdir))
    {
        // TODO: Better error handling
        fprintf(stderr, "Could not read CWD\n");
    }

    char *input_file_path = cli_args.filename;

    /* Initialization */
    /* TODO: this is placeholder code. Miniaudio objects have manual memory management, so we'd (probably) want to make these objects
       globals or heap-allocate them, depending on their size. */

    //ma_result result;
    ma_decoder decoder;
    ma_device_config device_conf;
    ma_device device;

    if (ma_decoder_init_file(input_file_path, NULL, &decoder))
    {
        fprintf(stderr, "Error trying to open file %s\n", input_file_path);
        return -2;
    }

    device_conf = ma_device_config_init(ma_device_type_playback);
    device_conf.playback.format = decoder.outputFormat;
    device_conf.playback.channels = decoder.outputChannels;
    device_conf.sampleRate = decoder.outputSampleRate;
    device_conf.dataCallback = data_callback;
    device_conf.pUserData = &decoder;

    if (ma_device_init(NULL, &device_conf, &device) != MA_SUCCESS)
    {
        fprintf(stderr, "Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return -3;
    }


    if (ma_device_start(&device) != MA_SUCCESS)
    {
        fprintf(stderr, "Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return -4;
    }

    if ((curr_dir_files_length = list_files("./", ~0 & ~FTYPE_UNKNOWN, &curr_dir_files)) < 0)
    {
        fprintf(stderr, "Could not get file list.\n");
    }

    printf("Playing %s, press enter to quit...\n", input_file_path);

    /* Platform vars */


    /* GLFW initialization */

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialized glfw\n");
        goto cleanup_and_exit;
    }
    glfwSetErrorCallback(error_callback);


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // TODO: remove magic numbers
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    win = glfwCreateWindow(default_width, default_height, "Visualozer", NULL, NULL);

    glfwSetWindowSizeCallback(win, window_size_callback);

    glfwMakeContextCurrent(win);
    glfwGetWindowSize(win, &glfw.width, &glfw.height);

    //glfwSetCursorPosCallback(win, cursor_position_callback);
    //if (!glfwSetMouseButtonCallback(win, mouse_button_callback))
    //    fprintf(stderr, "Error setting mouse button callback\n");


    /* Get openGL context */

    glViewport(0, 0, glfw.width, glfw.height);
    glewExperimental = 1;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to setup GLEW\n");
        goto cleanup_and_exit;
    }

    // TODO: Move this
#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

    ctx = nk_glfw3_init(&glfw, win, NK_GLFW3_INSTALL_CALLBACKS);

    {
        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(&glfw, &atlas);
        nk_glfw3_font_stash_end(&glfw);
    }


    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;


    while (!glfwWindowShouldClose(win) && !closing_window)
    {
        /* Input handling & sync */
        mouse_input = (const mouse_input_t){0};
        glfwWaitEvents();
        if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            mouse_input.state |= MOUSE_LEFT_CLICK;
        else
            mouse_input.state &= ~MOUSE_LEFT_CLICK;

        draw_gui();
    }

cleanup_and_exit:
    nk_glfw3_shutdown(&glfw);
    glfwTerminate();
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
    return 0;
}

void draw_gui()
{
    static bool first_frame = true;
    static uint64_t frame = 0;
    /* GUI */
    if (first_frame || !nk_window_is_closed(ctx, "WIP"))
    {

        // printf("--- Drawing frame %ld ---\n glfw.width = %d\n glfw.height = %d\n", frame++, glfw.width, glfw.height);
        nk_glfw3_new_frame(&glfw);
        if (nk_begin(ctx, "WIP", nk_rect(0, 0, glfw.width, glfw.height),
                     NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_CLOSABLE))
        {

            /* printf(" nkwsize = %f, %f\n", nkwsize.x, nkwsize.y); */
            /* if (floor(nkwsize.x) != glfw.width || floor(nkwsize.y) != glfw.height) */
            /* { */
            /*     glfwSetWindowSize(win, (float)nkwsize.x, (float)nkwsize.y); */
            /*     // Not needed anymore, callback will take care of updating these */
            /*     glfw.width = nkwsize.x; */
            /*     glfw.height = nkwsize.y; */
            /* } */
            /* else */
            if (window_drag_active && mouse_drag_initial_y <= 30)
            {
                // TODO: Make this read nuklear's style struct instead of hardcoding the 30px border
                double xpos, ypos;
                glfwGetCursorPos(win, &xpos, &ypos);

                delta_x = xpos - mouse_drag_initial_x;
                delta_y = ypos - mouse_drag_initial_y;

                int x, y;
                glfwGetWindowPos(win, &x, &y);
                glfwSetWindowPos(win, x + delta_x, y + delta_y);
            }
            nk_layout_row_dynamic(ctx, 0, 1);
            if (nk_tree_push(ctx, NK_TREE_TAB, "WIP", NK_MINIMIZED))
            {
                nk_label(ctx, "File picker goes here", NK_TEXT_LEFT);
                draw_file_picker();
                nk_tree_pop(ctx);
            }

        }
        else
        {
            // TODO: Error handling
            closing_window = true;
        }

        nk_end(ctx);
    }

    /* Draw */
    glViewport(0, 0, glfw.width, glfw.height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(bg.r, bg.g, bg.b, bg.a);
    /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state after
     * rendering the UI. */
    nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    glfwSwapBuffers(win);
    first_frame = false;

}

void draw_file_picker()
{
    nk_layout_row_dynamic(ctx, 500, 1);
    if (nk_group_begin(ctx, "File Picker", NK_WINDOW_BORDER))
    {
        nk_layout_row_begin(ctx, NK_DYNAMIC, 0, 2);
        nk_layout_row_push(ctx, 0.25f);
        nk_label(ctx, "Name", NK_TEXT_CENTERED);
        nk_layout_row_push(ctx, 0.75f);
        nk_label(ctx, "Placeholder", NK_TEXT_LEFT);
        nk_layout_row_end(ctx);

        for (size_t i = 0; i < curr_dir_files_length; ++i)
        {
            nk_layout_row_begin(ctx, NK_DYNAMIC, 0, 2);
            nk_layout_row_push(ctx, 0.25f);
            if (nk_button_label(ctx, curr_dir_files[i].filename))
            {
                enter_directory(curr_dir_files[i].filename);
                //fprintf(stdout, "topdir: %s\n", topdir);
                free(curr_dir_files);
                curr_dir_files = NULL;
                if ((curr_dir_files_length = list_files(topdir, ~0 & ~FTYPE_UNKNOWN, &curr_dir_files)) < 0)
                {
                    fprintf(stderr, "Could not get file list.\n");

                }
                else
                {
                    nk_layout_row_end(ctx);
                    goto early_exit_picker;
                }
            }
            nk_layout_row_push(ctx, 0.75f);
            nk_label(ctx, "Column 2", NK_TEXT_LEFT);
            nk_layout_row_end(ctx);
        }

    early_exit_picker:
        nk_group_end(ctx);
    }
}

/* Fills a fileinfo_t array with file properties. Also takes care of reserving
   the memory for such array. The starting position of the array is stored at
   *info.

  Return code: the number of files found, or a negative number on error
  TODO: Cache / filesystem watch the result of this function
*/
int list_files(char *path, filetype_t extension_mask, fileinfo_t **info)
{
    DIR *dir = opendir(path);
    struct dirent *ent;
    int retvalue = 0;

    if (!dir) return -1;

    int c = 0;

    while ((ent = readdir(dir)))
    {
        //fprintf(stdout, "Test %d with file %s\n", ++c, ent->d_name);
        bool file_accepted = false;
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
                strcpy(fullpath, topdir);
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
    closedir(dir);
    return retvalue;
}

int enter_directory(char *relpath)
{
    char aux[PATH_MAX+1];
    strcpy(aux, topdir);
    strcat(aux, "/");
    strcat(aux, relpath);
    char *p = realpath(aux, topdir);
    if (!p)
        return errno;
    else
        return 0;
}

bool str_ends_with(const char *haystack, const char *needle)
{
    size_t needle_length = strlen(needle);
    size_t haystack_length = strlen(haystack);
    char *p = strstr(haystack, needle);

    return (p && (haystack + haystack_length) - p == (ptrdiff_t)needle_length);
}

int get_current_dir(char *destination)
{
    char *p = getcwd(destination, PATH_MAX+1);
    if (!p)
        return errno;
    else
        return 0;
}
