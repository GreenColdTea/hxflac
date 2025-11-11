/**
 * my ass c++ coding skills less gooooo
 */

#include "hxflac.hpp"
#include "flac_decoder.h"
#include <cstdlib>
#include <cstring>
#include <cctype>

#ifdef __cplusplus
extern "C" 
{
    #endif
    #include <FLAC/export.h>
    #include <FLAC/metadata.h>
    
#ifdef __cplusplus
}
#endif

static int string_case_compare(const char* s1, const char* s2) 
{
    // TODO: hxflac support for non-Windows platforms?
    #ifdef _WIN32
        return _stricmp(s1, s2);
    #else
        return strcasecmp(s1, s2);
    #endif
}

static const char* get_flac_version() 
{
    return FLAC__VERSION_STRING;
}

typedef struct {
    const FLAC__byte* data;
    size_t size;
    size_t pos;
} MemoryReader;

static size_t memory_read(void* ptr, size_t size, size_t nmemb, FLAC__IOHandle handle) 
{
    MemoryReader* reader = (MemoryReader*)handle;
    size_t bytes_to_read = size * nmemb;
    
    if (reader->pos >= reader->size) return 0;
    if (reader->pos + bytes_to_read > reader->size) 
    {
        bytes_to_read = reader->size - reader->pos;
    }
    
    memcpy(ptr, reader->data + reader->pos, bytes_to_read);
    reader->pos += bytes_to_read;
    return bytes_to_read / size;
}

static int memory_seek(FLAC__IOHandle handle, FLAC__int64 offset, int whence) 
{
    MemoryReader* reader = (MemoryReader*)handle;
    
    switch (whence) 
    {
        case SEEK_SET:
            if (offset < 0 || (size_t)offset > reader->size) return -1;
            reader->pos = (size_t)offset;
            break;
        case SEEK_CUR:
            if (reader->pos + offset < 0 || reader->pos + offset > reader->size) return -1;
            reader->pos += (size_t)offset;
            break;
        case SEEK_END:
            if (offset > 0 || reader->size + offset < 0) return -1;
            reader->pos = reader->size + (size_t)offset;
            break;
        default:
            return -1;
    }
    return 0;
}

static FLAC__int64 memory_tell(FLAC__IOHandle handle) 
{
    MemoryReader* reader = (MemoryReader*)handle;
    return reader->pos;
}

static int memory_eof(FLAC__IOHandle handle) 
{
    MemoryReader* reader = (MemoryReader*)handle;
    return reader->pos >= reader->size;
}

static int extract_flac_metadata(const unsigned char* input_data, size_t input_length, const char** title, const char** artist, 
    const char** album, const char** genre, const char** year, const char** track, const char** comment) 
{
    *title = *artist = *album = *genre = *year = *track = *comment = nullptr;
    
    FLAC__Metadata_Chain* chain = FLAC__metadata_chain_new();
    if (!chain) return 0;
    
    MemoryReader reader;
    reader.data = input_data;
    reader.size = input_length;
    reader.pos = 0;
    
    FLAC__IOCallbacks callbacks;
    callbacks.read = memory_read;
    callbacks.seek = memory_seek;
    callbacks.tell = memory_tell;
    callbacks.eof = memory_eof;
    callbacks.write = nullptr;
    
    if (!FLAC__metadata_chain_read_with_callbacks(chain, &reader, callbacks)) 
    {
        FLAC__metadata_chain_delete(chain);
        return 0;
    }
    
    FLAC__Metadata_Iterator* iterator = FLAC__metadata_iterator_new();
    if (!iterator) 
    {
        FLAC__metadata_chain_delete(chain);
        return 0;
    }
    
    FLAC__metadata_iterator_init(iterator, chain);
    
    int metadata_found = 0;
    
    do 
    {
        if (FLAC__metadata_iterator_get_block_type(iterator) == FLAC__METADATA_TYPE_VORBIS_COMMENT) 
        {
            FLAC__StreamMetadata* block = FLAC__metadata_iterator_get_block(iterator);
            FLAC__StreamMetadata_VorbisComment* vorbis_comment = &block->data.vorbis_comment;
            
            for (unsigned i = 0; i < vorbis_comment->num_comments; i++) 
            {
                FLAC__StreamMetadata_VorbisComment_Entry entry = vorbis_comment->comments[i];
                char* comment_str = (char*)malloc(entry.length + 1);
                if (!comment_str) continue;
                
                memcpy(comment_str, entry.entry, entry.length);
                comment_str[entry.length] = '\0';
                char* equals = strchr(comment_str, '=');
                if (equals) 
                {
                    *equals = '\0';
                    char* field_name = comment_str;
                    char* field_value = equals + 1;
                    
                    if (string_case_compare(field_name, "TITLE") == 0 && !*title) 
                    {
                        *title = strdup(field_value);
                    }
                    else if (string_case_compare(field_name, "ARTIST") == 0 && !*artist) 
                    {
                        *artist = strdup(field_value);
                    }
                    else if (string_case_compare(field_name, "ALBUM") == 0 && !*album) 
                    {
                        *album = strdup(field_value);
                    }
                    else if (string_case_compare(field_name, "GENRE") == 0 && !*genre) 
                    {
                        *genre = strdup(field_value);
                    }
                    else if (string_case_compare(field_name, "DATE") == 0 && !*year) 
                    {
                        *year = strdup(field_value);
                    }
                    else if (string_case_compare(field_name, "TRACKNUMBER") == 0 && !*track) 
                    {
                        *track = strdup(field_value);
                    }
                    else if (string_case_compare(field_name, "COMMENT") == 0 && !*comment) 
                    {
                        *comment = strdup(field_value);
                    }
                }
                
                free(comment_str);
            }
            
            metadata_found = 1;
            break;
        }
    } while (FLAC__metadata_iterator_next(iterator));
    
    FLAC__metadata_iterator_delete(iterator);
    FLAC__metadata_chain_delete(chain);
    
    return metadata_found;
}

extern "C" 
{
    void flac_decode_from_memory(const unsigned char* input_data, int input_length, unsigned char** output_data, int* output_length,
        int* sample_rate,
        int* channels,
        int* bits_per_sample
    ) {
        size_t out_len;
        unsigned sr, ch, bps;
        
        int result = decode_flac_data(
            input_data,
            (size_t)input_length,
            output_data,
            &out_len,
            &sr,
            &ch,
            &bps
        );
        
        if (result && output_data && *output_data) {
            *output_length = (int)out_len;
            *sample_rate = (int)sr;
            *channels = (int)ch;
            *bits_per_sample = (int)bps;
        } else {
            *output_data = nullptr;
            *output_length = 0;
            *sample_rate = 0;
            *channels = 0;
            *bits_per_sample = 0;
        }
    }

    int flac_get_metadata(const unsigned char* input_data, int input_length, const char** title, const char** artist, 
        const char** album, const char** genre, const char** year, const char** track, const char** comment) 
    {
        return extract_flac_metadata(input_data, (size_t)input_length, title, artist, album, genre, year, track, comment);
    }

    void flac_free_result(unsigned char* data) {
        if (data) {
            free(data);
        }
    }

    void flac_free_string(const char* str) {
        if (str) {
            free((void*)str);
        }
    }

    void hxflac_to_bytes(const unsigned char* input_data, int input_length, unsigned char** output_data, int* output_length,
        int* sample_rate,
        int* channels,
        int* bits_per_sample) 
    {
        flac_decode_from_memory(input_data, input_length,output_data,
            output_length,
            sample_rate,
            channels,
            bits_per_sample
        );
    }

    int hxflac_get_metadata(const unsigned char* input_data, int input_length, const char** title, const char** artist, 
        const char** album, const char** genre, const char** year, const char** track, const char** comment) 
    {
        return flac_get_metadata(input_data, input_length, title, artist, album, genre, year, track, comment);
    }

    void hxflac_free_result(unsigned char* data) {
        flac_free_result(data);
    }

    void hxflac_free_string(const char* str) {
        flac_free_string(str);
    }

    const char* hxflac_get_version_string() {
        return get_flac_version();
    }
}