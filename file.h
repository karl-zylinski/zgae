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

enum struct FileEnding
{
    None, Zero
};

LoadedFile file_load(const char* filename, FileEnding ending);
bool file_write(void* data, unsigned size, const char* filename);
