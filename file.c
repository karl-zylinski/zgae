#include "file.h"
#include <stdio.h>
#include "memory.h"

int file_load(const char* filename, void** data, size_t* data_size)
{
    FILE* file_handle = fopen(filename, "rb");

    if (file_handle == NULL)
        return 0;

    fseek(file_handle, 0, SEEK_END);
    *data_size = ftell(file_handle);
    fseek(file_handle, 0, SEEK_SET);
    *data = mema(*data_size);
    fread(*data, 1, *data_size, file_handle);
    fclose(file_handle);
    return 1;
}

int file_load_str(const char* filename, char** data, size_t* str_len)
{
    FILE* file_handle = fopen(filename, "rb");

    if (file_handle == NULL)
        return 0;

    fseek(file_handle, 0, SEEK_END);
    (*str_len) = ftell(file_handle);
    fseek(file_handle, 0, SEEK_SET);
    *data = mema((*str_len) + 1);
    fread(*data, 1, *str_len, file_handle);
    (*data)[*str_len] = '\0';
    fclose(file_handle);
    return 1;
}