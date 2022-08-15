#include "main.h"

/* in-file function signatures */
void draw_gui();
void draw_file_picker();
int enter_directory(char *relpath);
bool open_music_file(char *relpath);


int list_files(char *path, filetype_t extension_mask, fileinfo_t **info);
bool str_ends_with(const char *haystack, const char *needle);
int exit_directory();

/* Global vars */

const filetype_t FTYPE_AUDIO = FTYPE_WAV | FTYPE_MP3 | FTYPE_FLAC | FTYPE_VORBIS;

const int32_t default_width = 1024;
const int32_t default_height = 720;

static double delta_x = 0;
static double delta_y = 0;

nk_bool show_hidden_files;

int window_drag_active;
int32_t mouse_drag_initial_x;
int32_t mouse_drag_initial_y;

bool dragging_window = false;
bool closing_window = false;
GLFWwindow *win = NULL;
int width;
int height;

fileinfo_t *curr_dir_files;
int32_t curr_dir_files_length;
char topdir[PATH_MAX+1] = ".";


struct nk_glfw glfw = {0};
struct nk_context *ctx;
struct nk_colorf bg;

/* Miniaudio globals */
ma_decoder decoder;
ma_device_config device_conf;
ma_device device;


// TODO: Make a nicer error callback for glfw
static void error_callback(int e, const char *d)
{
    fprintf(stderr, "Error %d: %s\n", e, d);
}

static void window_size_callback(__attribute__((unused))GLFWwindow *window, int new_width, int new_height)
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
    printf("Usage: visualozer [-f <file>]\n");
}


int main(int argc, char *argv[])
{
    // TODO: Set locale before initialization ?

    /* Argument parsing */
    cli_args_t cli_args = {};

    int errorcode = parse_args(argc, argv, &cli_args);

    if (errorcode || cli_args.print_help)   // just print help and exit
    {
        print_usage();
        return errorcode;
    }

    if (get_current_dir(topdir))
    {
        // TODO: Better error handling
        fprintf(stderr, "Could not read CWD\n");
    }

    if (cli_args.filename)
    {
        char *input_file_path = cli_args.filename;
        open_music_file(input_file_path);
    }



    if ((curr_dir_files_length = list_files("./", ~0 & ~FTYPE_UNKNOWN, &curr_dir_files)) < 0)
    {
        fprintf(stderr, "Could not get file list.\n");
    }

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
    static uint64_t frame = 0;
    /* GUI */
    if (!frame++ || !nk_window_is_closed(ctx, "WIP"))
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
                nk_layout_row_template_begin(ctx, 30);
                nk_layout_row_template_push_variable(ctx, 1);
                nk_layout_row_template_push_static(ctx, 100);
                nk_layout_row_template_end(ctx);
                nk_label(ctx, topdir, NK_TEXT_LEFT);
                nk_checkbox_label(ctx, "Show hidden", &show_hidden_files);
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
}

// TODO: Alphabetical / datestamp ordering of files
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

        for (int i = 0; i < curr_dir_files_length; ++i)
        {
            bool show_file = (show_hidden_files || !strcmp(curr_dir_files[i].filename, "..") || curr_dir_files[i].filename[0] != '.');
            if (show_file)
            {
                nk_layout_row_begin(ctx, NK_DYNAMIC, 0, 2);
                nk_layout_row_push(ctx, 0.25f);
                if (nk_button_label(ctx, curr_dir_files[i].filename))
                {
                    if (curr_dir_files[i].type & FTYPE_DIR)
                    {
                        /* If clicked file is a directory, try to get inside */
                        enter_directory(curr_dir_files[i].filename);
                        //fprintf(stdout, "topdir: %s\n", topdir);
                        curr_dir_files_length = list_files(topdir, ~0 & ~FTYPE_UNKNOWN, &curr_dir_files);
                        if (curr_dir_files_length < 0)
                        {
                            fprintf(stderr, "Could not get file list for %s.\n", topdir);
                            exit_directory();
                            curr_dir_files_length = list_files(topdir, ~0 & ~FTYPE_UNKNOWN, &curr_dir_files);
                        }
                    }
                    else if (curr_dir_files[i].type & FTYPE_AUDIO)
                    {
                        char filepath[PATH_MAX+1];
                        strcpy(filepath, topdir);
                        strcat(filepath, "/");
                        strcat(filepath, curr_dir_files[i].filename);
                        open_music_file(filepath);
                    }

                }
                nk_layout_row_push(ctx, 0.75f);
                nk_label(ctx, "Column 2", NK_TEXT_LEFT);
                nk_layout_row_end(ctx);
            }
        }

        nk_group_end(ctx);
    }
}



int enter_directory(char *relpath)
{
    char aux[PATH_MAX+1];
    strcpy(aux, topdir);
    strcat(aux, "/");
    strcat(aux, relpath);
    return absolute_path(topdir, aux);
}

int exit_directory()
{
    char aux[PATH_MAX+1];
    strcpy(aux, topdir);
    strcat(aux, "/../");
    return absolute_path(topdir, aux);
}

bool open_music_file(char *path)
{
    char truepath[PATH_MAX];
    ma_result errcode;
    if (path_is_absolute(path))
    {
        strcpy(truepath, path);
    }
    else
    {
        // TODO: Error checking
        absolute_path(truepath, path);
    }

    /* NOTE: When creating a MA device, its config is immutable. We may be able to reuse it if the decoder config
       is identical but creating a new one seems saner. */
    ma_device_uninit(&device);

    if (ma_decoder_uninit(&decoder))
    {
        fprintf(stderr, "Error trying to free MA decoder\n");
        return false;
    }

    if ((errcode = ma_decoder_init_file(truepath, NULL, &decoder)) != MA_SUCCESS)
    {
        fprintf(stderr, "Error trying to open decoder for file %s\nERRCODE: %d\n", truepath, errcode);
        return false;
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
        return false;
    }

    if (ma_device_start(&device) != MA_SUCCESS)
    {
        fprintf(stderr, "Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return false;
    }
    return true;
}
