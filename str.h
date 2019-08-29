#pragma once

void str_app(char* s, char* app);
void str_app_s(char* s, char* app, u32 app_len);
bool str_eql(char* s1, char* s2);
i32 str_eql_arr(char* s, char** comp_arr, u32 comp_arr_num);
char* str_copy(char* s);
char* str_copy_s(char* s, u32 len);
hash64 str_hash(char* str);