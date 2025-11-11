#include "flac_decoder.h"
#include <stdio.h>
#include <string.h>

static const size_t INITIAL_BUFFER_CAPACITY = 4 * 1024 * 1024; //4MB
static const size_t MIN_BUFFER_CAPACITY = 128 * 1024; //128KB
static const size_t MAX_BUFFER_CAPACITY = 128 * 1024 * 1024; //128MB

#ifdef FLAC_DEBUG
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

static size_t get_bytes_per_sample(unsigned bits_per_sample) {
    return (bits_per_sample + 7) / 8;
}

static void convert_samples_16bit(const FLAC__int32* const* buffers, unsigned channels, unsigned samples, unsigned char** output) 
{
    for (unsigned i = 0; i < samples; i++) {
        for (unsigned ch = 0; ch < channels; ch++) {
            FLAC__int16 sample = (FLAC__int16)buffers[ch][i];
            *(*output)++ = (unsigned char)(sample & 0xFF);
            *(*output)++ = (unsigned char)((sample >> 8) & 0xFF);
        }
    }
}

static void convert_samples_24bit(const FLAC__int32* const* buffers, unsigned channels, unsigned samples, unsigned char** output) 
{
    for (unsigned i = 0; i < samples; i++) {
        for (unsigned ch = 0; ch < channels; ch++) {
            FLAC__int32 sample = buffers[ch][i];
            *(*output)++ = (unsigned char)(sample & 0xFF);
            *(*output)++ = (unsigned char)((sample >> 8) & 0xFF);
            *(*output)++ = (unsigned char)((sample >> 16) & 0xFF);
        }
    }
}

static void convert_samples_generic(const FLAC__int32* const* buffers, unsigned channels, unsigned samples, unsigned bits_per_sample, unsigned char** output) 
{
    const size_t bytes_per_sample = get_bytes_per_sample(bits_per_sample);
    
    for (unsigned i = 0; i < samples; i++) {
        for (unsigned ch = 0; ch < channels; ch++) {
            FLAC__int32 sample = buffers[ch][i];
            
            for (size_t byte = 0; byte < bytes_per_sample; byte++) {
                *(*output)++ = (unsigned char)((sample >> (byte * 8)) & 0xFF);
            }
        }
    }
}

static int ensure_buffer_capacity(flac_decoder_context* context, size_t required_capacity) 
{
    if (context->pcm_buffer_size + required_capacity <= context->pcm_buffer_capacity) {
        return 1;
    }

    size_t new_capacity = context->pcm_buffer_capacity;
    while (new_capacity < context->pcm_buffer_size + required_capacity) {
        new_capacity *= 2;
        if (new_capacity > MAX_BUFFER_CAPACITY) {
            break;
        }
    }

    if (new_capacity < context->pcm_buffer_size + required_capacity) {
        new_capacity = context->pcm_buffer_size + required_capacity;
    }

    if (new_capacity > MAX_BUFFER_CAPACITY) {
        context->error = 1;
        snprintf(context->error_message, sizeof(context->error_message),
                "Buffer capacity exceeded maximum limit of %zu bytes (required: %zu)", 
                MAX_BUFFER_CAPACITY, context->pcm_buffer_size + required_capacity);
        DEBUG_PRINT("[HXFLAC] %s\n", context->error_message);
        return 0;
    }

    unsigned char* new_buffer = (unsigned char*)realloc(context->pcm_buffer, new_capacity);
    if (!new_buffer) {
        context->error = 1;
        snprintf(context->error_message, sizeof(context->error_message),
                "Memory allocation failed for %zu bytes", new_capacity);
        DEBUG_PRINT("[HXFLAC] %s\n", context->error_message);
        return 0;
    }

    DEBUG_PRINT("Buffer reallocated: %zu -> %zu bytes\n", context->pcm_buffer_capacity, new_capacity);
    context->pcm_buffer = new_buffer;
    context->pcm_buffer_capacity = new_capacity;
    return 1;
}

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffers[], void* client_data) 
{
    flac_decoder_context* context = (flac_decoder_context*)client_data;
    
    const unsigned channels = frame->header.channels;
    const unsigned samples = frame->header.blocksize;
    const unsigned bits_per_sample = frame->header.bits_per_sample;

    DEBUG_PRINT("Frame: %u samples, %u channels, %u bps, %u Hz\n", 
                samples, channels, bits_per_sample, frame->header.sample_rate);

    if (context->sample_rate == 0) {
        context->sample_rate = frame->header.sample_rate;
        context->channels = channels;
        context->bits_per_sample = bits_per_sample;
    }

    const size_t bytes_per_sample = get_bytes_per_sample(bits_per_sample);
    const size_t frame_size = samples * channels * bytes_per_sample;
    
    if (!ensure_buffer_capacity(context, frame_size)) {
        DEBUG_PRINT("[HXFLAC] Failed to ensure buffer capacity for frame of %zu bytes\n", frame_size);
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    unsigned char* output = context->pcm_buffer + context->pcm_buffer_size;
    switch (bits_per_sample) {
        case 16:
            convert_samples_16bit(buffers, channels, samples, &output);
            break;
        case 24:
            convert_samples_24bit(buffers, channels, samples, &output);
            break;
        default: 
            //for future flacs or whatever lol
            DEBUG_PRINT("Using generic conversion for %u bits per sample\n", bits_per_sample);
            convert_samples_generic(buffers, channels, samples, bits_per_sample, &output);
            break;
    }

    context->pcm_buffer_size += frame_size;
    DEBUG_PRINT("Decoded frame: %zu total bytes so far\n", context->pcm_buffer_size);
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void metadata_callback(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data) 
{
    flac_decoder_context* context = (flac_decoder_context*)client_data;
    
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        context->sample_rate = metadata->data.stream_info.sample_rate;
        context->channels = metadata->data.stream_info.channels;
        context->bits_per_sample = metadata->data.stream_info.bits_per_sample;
        
        DEBUG_PRINT("Stream info: %u Hz, %u channels, %u bps, %lu total samples\n",
                   context->sample_rate, context->channels, context->bits_per_sample,
                   metadata->data.stream_info.total_samples);
    }
}

void error_callback(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data) 
{
    flac_decoder_context* context = (flac_decoder_context*)client_data;
    context->error = 1;
    snprintf(context->error_message, sizeof(context->error_message),
             "FLAC decoder [HXFLAC] %s", FLAC__StreamDecoderErrorStatusString[status]);
    DEBUG_PRINT("ERROR CALLBACK: %s\n", context->error_message);
}

FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes, void* client_data) 
{
    flac_decoder_context* context = (flac_decoder_context*)client_data;
    
    if (context->input_position >= context->input_length) {
        *bytes = 0;
        DEBUG_PRINT("READ: End of stream reached\n");
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }

    const size_t bytes_available = context->input_length - context->input_position;
    const size_t bytes_to_read = (*bytes < bytes_available) ? *bytes : bytes_available;

    memcpy(buffer, context->input_data + context->input_position, bytes_to_read);
    context->input_position += bytes_to_read;
    *bytes = bytes_to_read;

    DEBUG_PRINT("READ: %zu bytes (position: %zu/%zu)\n", bytes_to_read, context->input_position, context->input_length);
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

int decode_flac_data(const unsigned char* input_data, size_t input_length, unsigned char** output_data, size_t* output_length, unsigned* sample_rate, unsigned* channels, unsigned* bits_per_sample) 
{
    DEBUG_PRINT("Starting FLAC decoding: %zu bytes input\n", input_length);
    
    if (input_length < 4 || memcmp(input_data, "fLaC", 4) != 0) {
        fprintf(stderr, "[HXFLAC] Not a valid FLAC file (missing fLaC signature)\n");
        return 0;
    }

    FLAC__StreamDecoder* decoder = FLAC__stream_decoder_new();
    if (!decoder) {
        fprintf(stderr, "[HXFLAC] Unable to create FLAC decoder\n");
        return 0;
    }

    flac_decoder_context context = {0};
    context.input_data = input_data;
    context.input_length = input_length;
    context.input_position = 0;
    
    context.pcm_buffer_capacity = INITIAL_BUFFER_CAPACITY;
    context.pcm_buffer = (unsigned char*)malloc(context.pcm_buffer_capacity);
    
    if (!context.pcm_buffer) {
        fprintf(stderr, "[HXFLAC] Memory allocation failed for output buffer\n");
        FLAC__stream_decoder_delete(decoder);
        return 0;
    }

    DEBUG_PRINT("Initialized decoder with %zu byte buffer\n", context.pcm_buffer_capacity);

    FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_stream(
        decoder,
        read_callback,
        NULL, //seek_callback
        NULL, //tell_callback
        NULL, //length_callback
        NULL, //eof_callback
        write_callback,
        metadata_callback,
        error_callback,
        &context
    );

    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        fprintf(stderr, "[HXFLAC] Initializing decoder: %s\n", 
                FLAC__StreamDecoderInitStatusString[init_status]);
        free(context.pcm_buffer);
        FLAC__stream_decoder_delete(decoder);
        return 0;
    }

    DEBUG_PRINT("Decoder initialized successfully, starting processing...\n");
    
    FLAC__bool success = FLAC__stream_decoder_process_until_end_of_stream(decoder);

    if (!success) {
        FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(decoder);
        fprintf(stderr, "[HXFLAC] Decoding failed: %s\n", 
                FLAC__StreamDecoderStateString[state]);
        
        if (context.error) {
            fprintf(stderr, "Context [HXFLAC] %s\n", context.error_message);
        }
    }

    FLAC__stream_decoder_finish(decoder);
    FLAC__stream_decoder_delete(decoder);

    if (context.error || !success || context.pcm_buffer_size == 0) {
        DEBUG_PRINT("Decoding failed or produced no data. [HXFLAC] %d, Success: %d, Output size: %zu\n", 
                   context.error, success, context.pcm_buffer_size);
        free(context.pcm_buffer);
        return 0;
    }

    DEBUG_PRINT("Decoding successful: %zu bytes output, %u Hz, %u channels, %u bps\n",
               context.pcm_buffer_size, context.sample_rate, context.channels, context.bits_per_sample);

    *output_data = context.pcm_buffer;
    *output_length = context.pcm_buffer_size;
    *sample_rate = context.sample_rate;
    *channels = context.channels;
    *bits_per_sample = context.bits_per_sample;

    return 1;
}