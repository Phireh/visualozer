/* STDLIB includes */
#include <stdio.h>
#include <getopt.h>

/* Misc. includes */
#ifdef __linux__
#include <linux/limits.h> // for PATH_MAX
#else
#include <limits.h> // TODO: check if Mac actually as PATH_MAX here
#endif

/* Miniaudio includes */
#include "miniaudio_config.h"
#define MINIAUDIO_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function" // ignore spureous warnings from 3rd party libs
#include "miniaudio.h"
#pragma GCC diagnostic pop


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

    ma_result result;
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
    getchar();


    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
    return 0;
}
