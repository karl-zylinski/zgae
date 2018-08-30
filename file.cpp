#include "file.h"
#include <stdio.h>
#include "memory.h"

LoadedFile file_load(const char* filename, FileEnding ending)
{
    FILE* file_handle = fopen(filename, "rb");

    if (file_handle == nullptr)
        return {false};

    fseek(file_handle, 0, SEEK_END);
    unsigned filesize = ftell(file_handle);
    unsigned buffersize = filesize + (ending == FileEnding::Zero ? 1 : 0);
    fseek(file_handle, 0, SEEK_SET);

    if (filesize == 0)
        return {false};

    unsigned char* data = (unsigned char*)zalloc(buffersize);

    if (data == nullptr)
        return {false};

    fread(data, 1, filesize, file_handle);

    if (ending == FileEnding::Zero)
        data[filesize] = 0;

    fclose(file_handle);
    File file = {};
    file.data = data;
    file.size = buffersize;
    return {true, file};
}

bool file_write(void* data, unsigned size, const char* filename)
{
    FILE* file_handle = fopen(filename, "wb");

    if (file_handle == nullptr)
        return false;

    fwrite(data, 1, size, file_handle);
    fclose(file_handle);
    return true;
}
