#include "str.h"
#include <string.h>
#include "memory.h"

void str_app(char* s, const char* to_append)
{
    str_app_s(s, to_append, strlen(to_append));
}

void str_app_s(char* s, const char* to_append, size_t to_append_len)
{
    size_t s_len = strlen(s);
    size_t tot_len = s_len + to_append_len;
    memra(s, tot_len + 1);
    memcpy(s + s_len, to_append, to_append_len);
    s[tot_len] = 0;
}

char* str_copy(const char* s)
{
    return str_copy_s(s, strlen(s));
}

char* str_copy_s(const char* s, size_t size)
{
    char* ns = mema(size + 1);
    memcpy(ns, s, size);
    ns[size] = 0;
    return ns;
}

int64_t str_hash(const char* s)
{
    size_t len = strlen(s);
    int64_t seed = 0;

    const int64_t m = 0xc6a4a7935bd1e995ULL;
    const uint32_t r = 47;

    int64_t h = seed ^ (len * m);

    const int64_t * data = (const int64_t *)s;
    const int64_t * end = data + (len / 8);

    while (data != end)
    {
        int64_t k = *data++;
        
        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const uint8_t* data2 = (const uint8_t*)data;

    switch (len & 7)
    {
    case 7: h ^= ((int64_t)data2[6]) << 48;
    case 6: h ^= ((int64_t)data2[5]) << 40;
    case 5: h ^= ((int64_t)data2[4]) << 32;
    case 4: h ^= ((int64_t)data2[3]) << 24;
    case 3: h ^= ((int64_t)data2[2]) << 16;
    case 2: h ^= ((int64_t)data2[1]) << 8;
    case 1: h ^= ((int64_t)data2[0]);
        h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}