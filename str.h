#pragma once

void str_app(char* s, const char* app);
void str_app_s(char* s, const char* app, u32 app_len);
bool str_eql(const char* s1, const char* s2);
i32 str_eql_arr(const char* s, const char** comp_arr, u32 comp_arr_num);
char* str_copy(const char* s);
char* str_copy_s(const char* s, u32 len);
i64 str_hash(const char* str);