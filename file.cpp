#include "file.h"
#include <stdio.h>
#include "memory.h"

FileLoadResult file_load(char* filename, FileLoadMode mode)
{
    FILE* file_handle = fopen(filename, "rb");

    if (!file_handle)
    {
        FileLoadResult r = {.ok = false};
        return r;
    }

    fseek(file_handle, 0, SEEK_END);
    u64 s = ftell(file_handle);
    fseek(file_handle, 0, SEEK_SET);
    void* d = mema(s + (mode == FileLoadMode::NullTerminated ? 1 : 0));
    fread(d, 1, s, file_handle);
    fclose(file_handle);

    if (mode == FileLoadMode::NullTerminated)
        ((char*)d)[s] = '\0';

    FileLoadResult r = {
        .ok = true,
        .data = d,
        .data_size = s
    };

    return r;
}
