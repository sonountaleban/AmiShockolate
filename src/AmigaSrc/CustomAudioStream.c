#ifdef __AROS__
#ifdef USE_SDL

#include "CustomAudioStream.h"

CustomAudioStream* NewAudioStream(const Uint16 src_format, const Uint8 src_channels, const int src_rate, const Uint16 dst_format, const Uint8 dst_channels, const int dst_rate)
{
    CustomAudioStream* stream = (CustomAudioStream*)malloc(sizeof(CustomAudioStream));
    stream->capacity = 262144;
    stream->buffer = (Uint8*)malloc(sizeof(Uint8) * stream->capacity);
    stream->write_pos = 0;
    stream->read_pos = 0;
    stream->mutex = SDL_CreateMutex();

    //memset(stream->buffer, 0, sizeof(Uint8) * stream->capacity);

    // Store source layout details
    stream->src_spec.format = src_format;
    stream->src_spec.channels = src_channels;
    stream->src_spec.freq = src_rate;

    // Store destination layout details (matching your Mix_OpenAudio specs)
    stream->dst_spec.format = dst_format;
    stream->dst_spec.channels = dst_channels;
    stream->dst_spec.freq = dst_rate;

    return stream;
}

void DestroyAudioStream(CustomAudioStream* stream)
{
    if (stream)
    {
        SDL_DestroyMutex(stream->mutex);
        free(stream->buffer);
        free(stream);
    }
}

// Equivalent to SDL_AudioStreamPut: Adds raw audio bytes to the stream
int AudioStreamPut(CustomAudioStream* stream, const void* data, int len)
{
    if (!stream || len <= 0)
    {
        return 0;
    }

    SDL_LockMutex(stream->mutex);

    SDL_AudioCVT cvt;
    Uint8* final_data = (Uint8*)data;
    int final_len = len;
    BOOL data_was_converted = FALSE;

    // 1. Check if conversion is needed (e.g., 22kHz Mono U8 -> 48kHz Stereo S16)
    if (stream->src_spec.format != stream->dst_spec.format ||
        stream->src_spec.channels != stream->dst_spec.channels ||
        stream->src_spec.freq != stream->dst_spec.freq)
    {
        if (SDL_BuildAudioCVT(&cvt, stream->src_spec.format, stream->src_spec.channels, stream->src_spec.freq,
                              stream->dst_spec.format, stream->dst_spec.channels, stream->dst_spec.freq) > 0)
        {
            // Allocate expansion memory (len * len_mult)
            cvt.buf = (Uint8*)malloc(len * cvt.len_mult);
            cvt.len = len; // Set original input size

            memcpy(cvt.buf, data, len);

            if (SDL_ConvertAudio(&cvt) == 0) {
                final_data = cvt.buf;
                final_len = cvt.len_cvt; // CRUCIAL: Use the NEW converted length!
                data_was_converted = TRUE;
            } else {
                free(cvt.buf);

                SDL_UnlockMutex(stream->mutex);

                return 0; // Conversion failed
            }
        }
    }

    // 2. Write the final data into the circular ring buffer
    size_t available_space = stream->capacity - (stream->write_pos - stream->read_pos);

    // If the converted audio is larger than our remaining buffer space, clamp it
    if ((size_t)final_len > available_space) {
        final_len = (int)available_space;
    }

    if (final_len > 0){
        size_t actual_write = stream->write_pos % stream->capacity;
        size_t space_to_end = stream->capacity - actual_write;

        if ((size_t)final_len <= space_to_end) {
            memcpy(&stream->buffer[actual_write], final_data, final_len);
        } else {
            memcpy(&stream->buffer[actual_write], final_data, space_to_end);
            memcpy(&stream->buffer[0], final_data + space_to_end, final_len - space_to_end);
        }

        stream->write_pos += final_len;
    }

    // Clean up temporary conversion allocation
    if (data_was_converted) {
        free(cvt.buf);
    }

    SDL_UnlockMutex(stream->mutex);

    // Return how many ORIGINAL bytes we successfully processed
    return data_was_converted ? len : final_len;
}

// Equivalent to SDL_AudioStreamGet: Extracts bytes to feed SDL_mixer
int AudioStreamGet(CustomAudioStream* stream, void* data, int len)
{
    SDL_LockMutex(stream->mutex);

    size_t available_data = stream->write_pos - stream->read_pos;
    if ((size_t)len > available_data)
    {
        len = (int)available_data;
    }

    if (len > 0)
    {
        size_t actual_read = stream->read_pos % stream->capacity;
        size_t data_to_end = stream->capacity - actual_read;

        if ((size_t)len <= data_to_end)
        {
            memcpy(data, &stream->buffer[actual_read], len);
        }
        else
        {
            memcpy(data, &stream->buffer[actual_read], data_to_end);
            memcpy((Uint8*)data + data_to_end, &stream->buffer[0], len - data_to_end);
        }
        stream->read_pos += len;
    }

    SDL_UnlockMutex(stream->mutex);

    return len;
}

// Equivalent to SDL_AudioStreamAvailable: Returns number of bytes ready to be read
int AudioStreamAvailable(CustomAudioStream* stream)
{
    SDL_LockMutex(stream->mutex);

    // The total unread bytes inside our tracking counter
    size_t available_bytes = stream->write_pos - stream->read_pos;

    SDL_UnlockMutex(stream->mutex);

    return (int)available_bytes;
}

// Equivalent to SDL_AudioStreamClear: Instantly flushes the buffer tracking points
void AudioStreamClear(CustomAudioStream* stream)
{
    SDL_LockMutex(stream->mutex);

    // Reset our track indices back to zero
    stream->write_pos = 0;
    stream->read_pos = 0;

    // Optional: Zero out the buffer memory to completely wipe remaining residue
    //memset(stream->buffer, 0, sizeof(Uint8) * stream->capacity);

    SDL_UnlockMutex(stream->mutex);
}

#endif
#endif
