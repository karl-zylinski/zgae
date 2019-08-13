#pragma once

bool file_load(const char* filename, void** data, uint64* data_size);
bool file_load_str(const char* filename, char** data, uint64* str_len);