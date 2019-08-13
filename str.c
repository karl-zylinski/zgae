#include "str.h"
#include <string.h>
#include "memory.h"

void str_app(char* s, const char* to_append)
{
    str_app_s(s, to_append, strlen(to_append));
}

void str_app_s(char* s, const char* to_append, uint64 to_append_len)
{
    uint64 s_len = strlen(s);
    uint64 tot_len = s_len + to_append_len;
    memra(s, tot_len + 1);
    memcpy(s + s_len, to_append, to_append_len);
    s[tot_len] = 0;
}

char* str_copy(const char* s)
{
    return str_copy_s(s, strlen(s));
}

char* str_copy_s(const char* s, uint64 size)
{
    char* ns = mema(size + 1);
    memcpy(ns, s, size);
    ns[size] = 0;
    return ns;
}

bool str_eql(const char* s1, const char* s2)
{
    return strcmp(s1, s2) == 0;
}

int64 str_hash(const char* s)
{
    uint64 len = strlen(s);
    int64 seed = 0;

    const int64 m = 0xc6a4a7935bd1e995ULL;
    const uint32 r = 47;

    int64 h = seed ^ (len * m);

    const int64 * data = (const int64 *)s;
    const int64 * end = data + (len / 8);

    while (data != end)
    {
        int64 k = *data++;
        
        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const uint8* data2 = (const uint8*)data;

    switch (len & 7)
    {
    case 7: h ^= ((int64)data2[6]) << 48;
    case 6: h ^= ((int64)data2[5]) << 40;
    case 5: h ^= ((int64)data2[4]) << 32;
    case 4: h ^= ((int64)data2[3]) << 24;
    case 3: h ^= ((int64)data2[2]) << 16;
    case 2: h ^= ((int64)data2[1]) << 8;
    case 1: h ^= ((int64)data2[0]);
        h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}