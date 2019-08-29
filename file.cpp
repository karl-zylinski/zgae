#include "file.h"
#include <stdio.h>
#include "memory.h"

FileLoadResult file_load(const char* filename, FileLoadMode mode)
{
    FILE* file_handle = fopen(filename, "rb");

    if (!file_handle)
        return {.ok = false};

    fseek(file_handle, 0, SEEK_END);
    u64 s = ftell(file_handle);
    fseek(file_handle, 0, SEEK_SET);
    void* d = mema(s + (mode == FILE_LOAD_MODE_NULL_TERMINATED ? 1 : 0));
    fread(d, 1, s, file_handle);
    fclose(file_handle);

    if (mode == FILE_LOAD_MODE_NULL_TERMINATED)
        ((char*)d)[s] = '\0';

    return {
        .ok = true,
        .data = d,
        .data_size = s
    };
}
