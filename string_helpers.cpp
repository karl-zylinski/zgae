#include "string_helpers.h"
#include <string.h>
#include "memory.h"

char* str_append(const char* original, const char* to_append, size_t to_append_len)
{
    auto org_len = strlen(original);
    auto append_len = to_append_len == (size_t)(-1) ? strlen(to_append) : to_append_len;
    auto new_string_len = org_len + append_len;
    char* new_string = (char*)zalloc(new_string_len + 1);
    memcpy(new_string, original, org_len);
    memcpy(new_string + org_len, to_append, append_len);
    new_string[new_string_len] = 0;
    return new_string;
}

char* str_append(const char* original, const char** to_append, size_t num_to_append)
{
    auto org_len = strlen(original);
    size_t to_append_len = 0;
    for (size_t i = 0; i < num_to_append; ++i)
        to_append_len += strlen(to_append[i]);
    auto new_string_len = org_len + to_append_len + 1;
    char* new_string = (char*)zalloc(new_string_len);
    strcpy(new_string, original);
    for (size_t i = 0; i < num_to_append; ++i)
        strcat(new_string, to_append[i]);
    return new_string;
}

char* str_copy(const char* original)
{
    auto org_len = strlen(original);
    auto new_string_len = org_len + 1;
    char* new_string = (char*)zalloc(new_string_len);
    strcpy(new_string, original);
    return new_string;
}

char* str_copy(const char* original, size_t size)
{
    auto org_len = size;
    auto new_string_len = org_len + 1;
    char* new_string = (char*)zalloc(new_string_len);
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


long long str_hash(const char* str)
{
    size_t len = strlen(str);
    long long seed = 0;

    const long long m = 0xc6a4a7935bd1e995ULL;
    const unsigned r = 47;

    long long h = seed ^ (len * m);

    const long long * data = (const long long *)str;
    const long long * end = data + (len / 8);

    while (data != end)
    {
        long long k = *data++;
        
        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char * data2 = (const unsigned char*)data;

    switch (len & 7)
    {
    case 7: h ^= ((long long)data2[6]) << 48;
    case 6: h ^= ((long long)data2[5]) << 40;
    case 5: h ^= ((long long)data2[4]) << 32;
    case 4: h ^= ((long long)data2[3]) << 24;
    case 3: h ^= ((long long)data2[2]) << 16;
    case 2: h ^= ((long long)data2[1]) << 8;
    case 1: h ^= ((long long)data2[0]);
        h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}