#pragma once

typedef struct file_load_result_t {
    bool ok;
    void* data;
    size_t data_size;
} file_load_result_t;

typedef enum file_load_mode_t {
    FILE_LOAD_MODE_DEFAULT,
    FILE_LOAD_MODE_NULL_TERMINATED
} file_load_mode_t;