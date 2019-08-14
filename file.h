#pragma once

bool file_load(const char* filename, void** data, uint64_t* data_size);
bool file_load_str(const char* filename, char** data, uint64_t* str_len);