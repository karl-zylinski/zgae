#pragma once

typedef struct FileLoadResult {
    bool ok;
    void* data;
    size_t data_size;
} FileLoadResult;

typedef enum FileLoadMode {
    FILE_LOAD_MODE_DEFAULT,
    FILE_LOAD_MODE_NULL_TERMINATED
} FileLoadMode;