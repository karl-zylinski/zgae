#pragma once
#include <stdint.h>
#include <string.h>

typedef struct jzon_key_value_pair jzon_key_value_pair_t;

typedef struct jzon_value
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
        jzon_key_value_pair_t* table_val;
        struct jzon_value* array_val;
    };
} jzon_value_t;

struct jzon_key_value_pair {
    char* key;
    long long key_hash;
    jzon_value_t val;
};

int jzon_parse(const char* input, jzon_value_t* output);
void jzon_free(jzon_value_t* val);
jzon_value_t* jzon_get(jzon_value_t* object, const char* key);
