#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif

void* FLAC__add_metadata_block(const void* metadata, void* encoder) {
    return NULL;
}

int FLAC__frame_add_header(const void* frame, void* bw) {
    return 0;
}

int FLAC__subframe_add_constant(const void* subframe, unsigned bits_per_sample, void* bw) {
    return 0;
}

int FLAC__subframe_add_fixed(const void* subframe, unsigned residual_samples,
                           unsigned bits_per_sample, void* bw) {
    return 0;
}

int FLAC__subframe_add_lpc(const void* subframe, unsigned residual_samples,
                         unsigned bits_per_sample, void* bw) {
    return 0;
}

int FLAC__subframe_add_verbatim(const void* subframe, unsigned bits_per_sample, void* bw) {
    return 0;
}