#include "file.h"
#include <stdio.h>
#include "memory.h"

file_load_result_t file_load(const char* filename, file_load_mode_t mode)
{
    FILE* file_handle = fopen(filename, "rb");

    if (file_handle == NULL)
    {
        file_load_result_t r = {.ok = false};
        return r;
    }

    fseek(file_handle, 0, SEEK_END);
    size_t s = ftell(file_handle);
    fseek(file_handle, 0, SEEK_SET);
    void* d = mema(s + (mode == FILE_LOAD_MODE_NULL_TERMINATED ? 1 : 0));
    fread(d, 1, s, file_handle);
    fclose(file_handle);

    if (mode == FILE_LOAD_MODE_NULL_TERMINATED)
        ((char*)d)[s] = '\0';

    file_load_result_t r = {
        .ok = true,
        .data = d,
        .data_size = s
    };

    return r;
}
