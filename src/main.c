#include "main.h"

/* Custom types */


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
GLFWwindow *win = NULL;


// TODO: Make a nicer error callback for glfw
static void error_callback(int e, const char *d)
{
    printf("Error %d: %s\n", e, d);
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


void print_usage()
{
    printf("Usage: visualozer -f <file>\n");
}


int main(int argc, char *argv[])
{
    // TODO: Set locale before initialization ?

    /* Argument parsing */

    char input_file_path[PATH_MAX] = {0};
    int help_flag = 0;

    int c;

    while (1)
    {
        int idx = 0;
        struct option long_options[] = {
            {"help", no_argument, &help_flag, 1},
            {"file", required_argument, 0, 'f'},
            {0,0,0,0}};
        c = getopt_long(argc, argv, "hf:", long_options, &idx);
        if (c == -1) break;

        switch (c)
        {
        case 0: // long option that sets a flag
            ;   // nothing we really wanna do here for now
            break;
        case 'h':
            help_flag = 1;
            break;
        case 'f':
            if (optarg)
                strcpy(input_file_path, optarg);
            else
                help_flag = 1;
            break;
        default:
            fprintf(stderr, "Error during argument parsing\n");
            print_usage();
            return -1;
            break;
        }
    }
    if (help_flag || !strlen(input_file_path))
    {
        print_usage();
        return -1;
    }

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


    printf("Playing %s, press enter to quit...\n", input_file_path);

    /* Platform vars */
    struct nk_glfw glfw = {0};
    int width = 0, height = 0;
    struct nk_context *ctx;
    struct nk_colorf bg;

    /* GLFW initialization */
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialized glfw\n");
        goto cleanup_and_exit;
    }

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
    glfwMakeContextCurrent(win);
    glfwGetWindowSize(win, &width, &height);
    //glfwSetCursorPosCallback(win, cursor_position_callback);
    //if (!glfwSetMouseButtonCallback(win, mouse_button_callback))
    //    fprintf(stderr, "Error setting mouse button callback\n");

    /* Get openGL context */

    glViewport(0, 0, width, height);
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


    bool first_frame = true;
    mouse_input = (const mouse_input_t){0};

    while (!glfwWindowShouldClose(win))
    {
        /* Input */
        glfwWaitEvents();
        if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            mouse_input.state |= MOUSE_LEFT_CLICK;
        else
            mouse_input.state &= ~MOUSE_LEFT_CLICK;

        /* GUI */
        if (first_frame || !nk_window_is_closed(ctx, "WIP"))
        {
            nk_glfw3_new_frame(&glfw);
            if (nk_begin(ctx, "WIP", nk_rect(0, 0, glfw.width, glfw.height),
                         NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_CLOSABLE))
            {

                nk_layout_row_dynamic(ctx, 25, 1);
                if (nk_tree_push(ctx, NK_TREE_TAB, "WIP", NK_MINIMIZED))
                {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_label(ctx, "File picker goes here", NK_TEXT_LEFT);
                    nk_tree_pop(ctx);
                }


            }
            else
            {
                goto cleanup_and_exit;
            }

            // TODO: Make this read nuklear's style struct instead of hardcoding the 30px border
            if (window_drag_active && mouse_drag_initial_y <= 30)
            {
                double xpos, ypos;
                glfwGetCursorPos(win, &xpos, &ypos);

                delta_x = xpos - mouse_drag_initial_x;
                delta_y = ypos - mouse_drag_initial_y;

                int x, y;
                glfwGetWindowPos(win, &x, &y);
                glfwSetWindowPos(win, x + delta_x, y + delta_y);
            }


            nk_end(ctx);
        }

        /* Draw */
        glfwGetWindowSize(win, &width, &height);
        glViewport(0, 0, width, height);
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



cleanup_and_exit:
    nk_glfw3_shutdown(&glfw);
    glfwTerminate();
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
    return 0;
}
