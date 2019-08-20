#pragma once

void str_app(char* s, const char* app);
void str_app_s(char* s, const char* app, size_t app_len);
bool str_eql(const char* s1, const char* s2);
int32_t str_eql_arr(const char* s, const char** comp_arr, uint32_t comp_arr_num);
char* str_copy(const char* s);
char* str_copy_s(const char* s, size_t len);
int64_t str_hash(const char* str);