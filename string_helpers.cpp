#include "string_helpers.h"
#include <string.h>
#include "memory.h"

char* str_append(Allocator* alloc, const char* original, const char* to_append)
{
    auto org_len = strlen(original);
    auto append_len = strlen(to_append);
    auto new_string_len = org_len + append_len + 1;
    char* new_string = (char*)alloc->alloc(new_string_len);
    strcpy(new_string, original);
    strcat(new_string, to_append);
    return new_string;
}

char* str_append(Allocator* alloc, const char* original, const char** to_append, size_t num_to_append)
{
    auto org_len = strlen(original);
    size_t to_append_len = 0;
    for (size_t i = 0; i < num_to_append; ++i)
        to_append_len += strlen(to_append[i]);
    auto new_string_len = org_len + to_append_len + 1;
    char* new_string = (char*)alloc->alloc(new_string_len);
    strcpy(new_string, original);
    for (size_t i = 0; i < num_to_append; ++i)
        strcat(new_string, to_append[i]);
    return new_string;
}

char* str_copy(Allocator* alloc, const char* original)
{
    auto org_len = strlen(original);
    auto new_string_len = org_len + 1;
    char* new_string = (char*)alloc->alloc(new_string_len);
    strcpy(new_string, original);
    return new_string;
}

char* str_copy(Allocator* alloc, const char* original, size_t size)
{
    auto org_len = size;
    auto new_string_len = org_len + 1;
    char* new_string = (char*)alloc->alloc(new_string_len);
    memcpy(new_string, original, size);
    new_string[new_string_len - 1] = 0;
    return new_string;
}

bool str_eql(const char* str1, const char* str2)
{
    return strcmp(str1, str2) == 0;
}

bool str_ends_with(const char* str, const char* ends)
{
    auto sl = strlen(str);
    auto el = strlen(ends);

    if (sl == 0 && el == 0)
        return true;

    if (el > sl)
        return false;

    for (size_t i = 0; i < sl && i < el; ++i)
    {
        char sc = str[sl - 1 - i];
        char ec = ends[el - 1 - i];

        if (sc != ec)
            return false;
    }

    return true;
}