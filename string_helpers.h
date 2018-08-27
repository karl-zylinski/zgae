#pragma once

char* str_append(const char* original, const char* to_append);
char* str_append(const char* original, const char** to_append, size_t num_to_append);
char* str_copy(const char* original);
char* str_copy(const char* original, size_t size);
bool str_eql(const char* str1, const char* str2);
bool str_ends_with(const char* str, const char* ends);