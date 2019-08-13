#pragma once

struct jzon_key_value_pair;

struct jzon_value
{
    bool is_string : 1;
    bool is_int : 1;
    bool is_float : 1;
    bool is_table : 1;
    bool is_array : 1;
    bool is_bool : 1;
    bool is_null : 1;
    uint64 size;

    union
    {
        char* string_val;
        int64 int_val;
        bool bool_val;
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

bool jzon_parse(const char* input, struct jzon_value* output);
void jzon_free(struct jzon_value* val);
struct jzon_value* jzon_get(struct jzon_value* object, const char* key);