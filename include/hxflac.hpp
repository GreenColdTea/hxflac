#ifndef HXFLAC_H
#define HXFLAC_H

#ifdef _WIN32
#define HXFLAC_API __declspec(dllexport)
#else
#define HXFLAC_API
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

    HXFLAC_API const char* hxflac_get_version_string();
    
    HXFLAC_API void hxflac_to_bytes(const unsigned char* data, int length, 
                        unsigned char** result_data, int* result_length,
                        int* sample_rate, int* channels, int* bits_per_sample);
    
    HXFLAC_API int hxflac_get_metadata(const unsigned char* input_data, int input_length,
                        const char** title, const char** artist, 
                        const char** album, const char** genre, 
                        const char** year, const char** track, 
                        const char** comment);
    
    HXFLAC_API void hxflac_free_string(const char* str);
    HXFLAC_API void hxflac_free_result(unsigned char* data);
    HXFLAC_API void flac_decode_from_memory(const unsigned char* input_data, int input_length,
                        unsigned char** output_data, int* output_length,
                        int* sample_rate, int* channels, int* bits_per_sample);
    
    HXFLAC_API int flac_get_metadata(const unsigned char* input_data, int input_length,
                        const char** title, const char** artist, 
                        const char** album, const char** genre, 
                        const char** year, const char** track, 
                        const char** comment);
    
    HXFLAC_API void flac_free_string(const char* str);
    
    HXFLAC_API void flac_free_result(unsigned char* data);

#ifdef __cplusplus
}
#endif

#endif