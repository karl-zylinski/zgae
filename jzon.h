#pragma once
#include <stdint.h>
#include <stdlib.h>

struct jzon_key_value_pair;

struct jzon_value
{
    uint32_t is_string : 1;
    uint32_t is_int : 1;
    uint32_t is_float : 1;
    uint32_t is_table : 1;
    uint32_t is_array : 1;
    uint32_t is_bool : 1;
    uint32_t is_null : 1;
    size_t size;

    union
    {
        char* string_val;
        int32_t int_val;
        uint32_t bool_val;
        float float_val;
        struct jzon_key_value_pair* table_val;
        struct jzon_value* array_val;
    };
};

struct jzon_key_value_pair {
    char* key;
    long long key_hash;
    struct jzon_value val;
};

int jzon_parse(const char* input, struct jzon_value* output);
void jzon_free(struct jzon_value* val);
struct jzon_value* jzon_get(struct jzon_value* object, const char* key);
