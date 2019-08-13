#pragma once
#include <stdlib.h>

int file_load(const char* filename, void** data, size_t* data_size);
int file_load_str(const char* filename, char** data, size_t* str_len);
