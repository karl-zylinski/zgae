#pragma once

void str_app(char* s, const char* app);
void str_app_s(char* s, const char* app, uint64_t app_len);
bool str_eql(const char* s1, const char* s2);
char* str_copy(const char* s);
char* str_copy_s(const char* s, uint64_t len);
int64_t str_hash(const char* str);