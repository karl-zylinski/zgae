#pragma once

fwd_struct(jzon_key_value_pair_t);

typedef struct jzon_value_t
{
    bool is_string : 1;
    bool is_int : 1;
    bool is_float : 1;
    bool is_table : 1;
    bool is_array : 1;
    bool is_bool : 1;
    bool is_null : 1;
    uint64_t size;

    union
    {
        char* string_val;
        int64_t int_val;
        bool bool_val;
        float float_val;
        jzon_key_value_pair_t* table_val;
        struct jzon_value_t* array_val;
    };
} jzon_value_t;

typedef struct jzon_key_value_pair_t {
    char* key;
    long long key_hash;
    jzon_value_t val;
} jzon_key_value_pair_t;

bool jzon_parse(const char* input, jzon_value_t* output);
void jzon_free(jzon_value_t* val);
jzon_value_t* jzon_get(jzon_value_t* object, const char* key);