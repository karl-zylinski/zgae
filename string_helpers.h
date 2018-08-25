#pragma once

struct Allocator;

char* str_append(Allocator* alloc, const char* original, const char* to_append);
char* str_append(Allocator* alloc, const char* original, const char** to_append, size_t num_to_append);
char* str_copy(Allocator* alloc, const char* original);
char* str_copy(Allocator* alloc, const char* original, size_t size);
bool str_eql(const char* str1, const char* str2);
bool str_ends_with(const char* str, const char* ends);