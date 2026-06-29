#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#ifdef __AROS__
#ifdef USE_SDL

#include <SDL/SDL.h>

typedef struct {
    Uint8* buffer;
    size_t write_pos;
    size_t read_pos;
    size_t capacity;
    SDL_mutex* mutex; // Essential because SDL_mixer runs on its own thread

    // Track audio specifications for on-the-fly filtering/conversion
    SDL_AudioSpec src_spec;
    SDL_AudioSpec dst_spec;
} CustomAudioStream;

extern CustomAudioStream* NewAudioStream(const Uint16 src_format, const Uint8 src_channels, const int src_rate, const Uint16 dst_format, const Uint8 dst_channels, const int dst_rate);
extern void DestroyAudioStream(CustomAudioStream* stream);
extern int AudioStreamPut(CustomAudioStream* stream, const void* data, int len);
extern int AudioStreamGet(CustomAudioStream* stream, void* data, int len);
extern int AudioStreamAvailable(CustomAudioStream* stream);
extern void AudioStreamClear(CustomAudioStream* stream);

#endif
#endif

#endif
