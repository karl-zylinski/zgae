#pragma once

struct FileLoadResult
{
    bool ok;
    void* data;
    size_t data_size;
};

enum struct FileLoadMode {
    Default, NullTerminated
};