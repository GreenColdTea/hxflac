#ifndef FLAC_DECODER_H
#define FLAC_DECODER_H

#include <FLAC/stream_decoder.h>

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define FLAC_DECODER_API __declspec(dllexport)
#else
#define FLAC_DECODER_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define FLAC_TARGET_BITS_PER_SAMPLE 16
#define FLAC_BYTES_PER_SAMPLE 2

typedef struct {
    const unsigned char* input_data;
    size_t input_length;
    size_t input_position;
    
    unsigned char* pcm_buffer;
    size_t pcm_buffer_size;
    size_t pcm_buffer_capacity;
    
    unsigned sample_rate;
    unsigned channels;
    unsigned bits_per_sample;
    
    int error;
    char error_message[256];
} flac_decoder_context;

FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *decoder,
    FLAC__byte buffer[],
    size_t *bytes,
    void *client_data);

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder,
    const FLAC__Frame *frame,
    const FLAC__int32 *const buffer[],
    void *client_data);

void metadata_callback(const FLAC__StreamDecoder *decoder,
    const FLAC__StreamMetadata *metadata,
    void *client_data);

void error_callback(const FLAC__StreamDecoder *decoder, 
    FLAC__StreamDecoderErrorStatus status,
    void *client_data);

FLAC_DECODER_API int decode_flac_data(const unsigned char* input_data, size_t input_length, 
    unsigned char** output_data, size_t* output_length,
    unsigned* sample_rate, unsigned* channels, 
    unsigned* bits_per_sample);

#ifdef __cplusplus
}
#endif

#endif