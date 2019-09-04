#pragma once

struct FileLoadResult
{
    bool ok;
    void* data;
    u64 data_size;
};

enum FileLoadMode
{
    FILE_LOAD_MODE_RAW, FILE_LOAD_MODE_NULL_TERMINATED
};

FileLoadResult file_load(const char* filename, FileLoadMode mode = FILE_LOAD_MODE_RAW);