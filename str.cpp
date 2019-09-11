#include "str.h"
#include <string.h>
#include "memory.h"

void str_app(char* s, const char* app)
{
    str_app_s(s, app, strlen(app));
}

void str_app_s(char* s, const char* app, u32 app_len)
{
    u32 s_len = strlen(s);
    u32 tot_len = s_len + app_len;
    memra(s, tot_len + 1);
    memcpy(s + s_len, app, app_len);
    s[tot_len] = 0;
}

char* str_copy(const char* s)
{
    return str_copy_s(s, strlen(s));
}

char* str_copy_s(const char* s, u32 size)
{
    char* ns = mema_tn(char, size + 1);
    memcpy(ns, s, size);
    ns[size] = 0;
    return ns;
}

bool str_eql(const char* s1, const char* s2)
{
    return strcmp(s1, s2) == 0;
}

i32 str_eql_arr(const char* s, const char** comp_arr, u32 comp_arr_num)
{
    for (u32 i = 0; i < comp_arr_num; ++i)
    {
        if (str_eql(s, comp_arr[i]))
            return i;
    }

    return -1;
}

i64 str_hash(const char* s)
{
    u32 len = strlen(s);
    i64 seed = 0;

    i64 m = 0xc6a4a7935bd1e995ULL;
    u32 r = 47;

    i64 h = seed ^ (len * m);

    i64 * data = (i64 *)s;
    i64 * end = data + (len / 8);

    while (data != end)
    {
        i64 k = *data++;
        
        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    u8* data2 = (u8*)data;

    switch (len & 7)
    {
    case 7: h ^= ((i64)data2[6]) << 48;
    case 6: h ^= ((i64)data2[5]) << 40;
    case 5: h ^= ((i64)data2[4]) << 32;
    case 4: h ^= ((i64)data2[3]) << 24;
    case 3: h ^= ((i64)data2[2]) << 16;
    case 2: h ^= ((i64)data2[1]) << 8;
    case 1: h ^= ((i64)data2[0]);
        h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}