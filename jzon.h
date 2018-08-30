#pragma once

struct JzonKeyValuePair;
typedef struct JzonKeyValuePair JzonKeyValuePair;

typedef struct JzonValue
{
    bool is_string : 1;
    bool is_int : 1;
    bool is_float : 1;
    bool is_table : 1;
    bool is_array : 1;
    bool is_bool : 1;
    bool is_null : 1;
    size_t size;

    union
    {
        char* string_val;
        int int_val;
        bool bool_val;
        float float_val;
        struct JzonKeyValuePair* table_val;
        struct JzonValue* array_val;
    };
} JzonValue;

struct JzonKeyValuePair {
    char* key;
    long long key_hash;
    JzonValue val;
};

struct JzonParseResult {
    bool valid;
    JzonValue output;
};

JzonParseResult jzon_parse(const char* input);
void jzon_free(JzonValue* val);
JzonValue* jzon_get(JzonValue* object, const char* key);
