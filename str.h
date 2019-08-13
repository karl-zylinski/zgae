#pragma once

#include <stdlib.h>
#include <stdint.h>

void str_app(char* s, const char* to_append);
void str_app_s(char* s, const char* to_append, size_t to_append_len);
int str_eql(const char* s1, const char* s2);
char* str_copy(const char* original);
char* str_copy_s(const char* original, size_t len);
int64_t str_hash(const char* str);
