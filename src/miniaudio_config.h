#ifndef MINIAUDIO_CONFIG_H
#define MINIAUDIO_CONFIG_H

/* This file bundles all the build-time config options for Miniaudio as written in http://miniaud.io/docs/manual/index.html#Building and the own miniaudio.h file.
   Generally, we are only interested in keeping the minimum amount of code for our target platforms and subsystems we actually use.
 */

/* Backend disabling */

// #define MA_NO_WASAPI                       //  Disables the WASAPI backend.
// #define MA_NO_DSOUND                       //  Disables the DirectSound backend.
// #define MA_NO_WINMM                        //  Disables the WinMM backend.
// #define MA_NO_ALSA                         //  Disables the ALSA backend.
// #define MA_NO_PULSEAUDIO                   //  Disables the PulseAudio backend.
// #define MA_NO_JACK                         //  Disables the JACK backend.
// #define MA_NO_COREAUDIO                    //  Disables the Core Audio backend.
// #define MA_NO_SNDIO                        //  Disables the sndio backend.
// #define MA_NO_AUDIO4                       //  Disables the audio(4) backend.
// #define MA_NO_OSS                          //  Disables the OSS backend.
// #define MA_NO_AAUDIO                       //  Disables the AAudio backend.
// #define MA_NO_OPENSL                       //  Disables the OpenSL backend.
// #define MA_NO_WEBAUDIO                     //  Disables the Web Audio backend.
// #define MA_NO_NULL                         //  Disables the null backend.

/* Or, alternatively, backend toggles */

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS   //  Disables all backends by default and requires MA_ENABLE_* to enable specific backends.
// #define MA_ENABLE_WASAPI                   //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the WASAPI backend.
// #define MA_ENABLE_DSOUND                   //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the DirectSound backend.
// #define MA_ENABLE_WINMM                    //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the WinMM backend.
#define MA_ENABLE_ALSA                     //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the ALSA backend.
#define MA_ENABLE_PULSEAUDIO               //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the PulseAudio backend.
#define MA_ENABLE_JACK                     //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the JACK backend.
#define MA_ENABLE_COREAUDIO                //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the Core Audio backend.
// #define MA_ENABLE_SNDIO                    //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the sndio backend.
// #define MA_ENABLE_AUDIO4                   //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the audio(4) backend.
#define MA_ENABLE_OSS                      //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the OSS backend.
// #define MA_ENABLE_AAUDIO                   //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the AAudio backend.
// #define MA_ENABLE_OPENSL                   //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the OpenSL backend.
// #define MA_ENABLE_WEBAUDIO                 //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the Web Audio backend.
// #define MA_ENABLE_NULL                     //  Used in conjunction with MA_ENABLE_ONLY_SPECIFIC_BACKENDS, enables the null backend.

/* Subsystem toggles */

// #define MA_NO_DECODING                     //  Disables decoding APIs.
#define MA_NO_ENCODING                     //  Disables encoding APIs.
// #define MA_NO_WAV                          //  Disables the built-in WAV decoder and encoder.
// #define MA_NO_FLAC                         //  Disables the built-in FLAC decoder.
// #define MA_NO_MP3                          //  Disables the built-in MP3 decoder.
// #define MA_NO_DEVICE_IO                    //  Disables playback and recording. This will disable ma_context and ma_device APIs. This is useful if you only want to use miniaudio's data conversion and/or decoding APIs.
// #define MA_NO_THREADING                    //  Disables the ma_thread, ma_mutex, ma_semaphore and ma_event APIs. This option is useful if you only need to use miniaudio for data conversion, decoding and/or encoding. Some families of APIs require threading which means the following options must also be set: MA_NO_DEVICE_IO
// #define MA_NO_GENERATION                   //  Disables generation APIs such a ma_waveform and ma_noise.

/* Optimization toggles */

// #define MA_NO_SSE2                         //  Disables SSE2 optimizations.
// #define MA_NO_AVX2                         //  Disables AVX2 optimizations.
// #define MA_NO_NEON                         //  Disables NEON optimizations.

/* Debugging, linking, misc. toggles */

// #define MA_NO_RUNTIME_LINKING              //  Disables runtime linking. This is useful for passing Apple's notarization process. When enabling this, you may need to avoid using -std=c89 or -std=c99 on Linux builds or else you may end up with compilation errors due to conflicts with timespec and timeval data types. You may need to enable this if your target platform does not allow runtime linking via dlopen().
// #define MA_DEBUG_OUTPUT                    //  Enable printf() output of debug logs (MA_LOG_LEVEL_DEBUG).
// #define MA_COINIT_VALUE                    //  Windows only. The value to pass to internal calls to CoInitializeEx(). Defaults to COINIT_MULTITHREADED.
// #define MA_API extern                      //  Controls how public APIs should be decorated. Default is extern.
// #define MA_DLL                             //  If set, configures MA_API to either import or export APIs depending on whether or not the implementation is being defined. If defining the implementation, MA_API will be configured to export. Otherwise it will be configured to import. This has no effect if MA_API is defined externally.

#endif
