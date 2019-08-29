#pragma once

struct FileLoadResult
{
    bool ok;
    void* data;
    u64 data_size;
};

enum struct FileLoadMode {
    Default, NullTerminated
};