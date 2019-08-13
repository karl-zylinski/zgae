#pragma once

void str_app(char* s, const char* to_append);
void str_app_s(char* s, const char* to_append, uint64 to_append_len);
bool str_eql(const char* s1, const char* s2);
char* str_copy(const char* original);
char* str_copy_s(const char* original, uint64 len);
int64 str_hash(const char* str);