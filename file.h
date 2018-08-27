#pragma once

struct File
{
    unsigned char* data;
    unsigned size;
};

struct LoadedFile
{
    bool valid;
    File file;
};

LoadedFile file_load(const char* filename);
bool file_write(void* data, unsigned size, const char* filename);
