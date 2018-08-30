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
    unsigned size;

    union
    {
        char* string_val;
        int int_val;
        bool bool_val;
        float float_val;
        struct JzonKeyValuePair** table_val;
        struct JzonValue** array_val;
    };
} JzonValue;

struct JzonKeyValuePair {
    char* key;
    long long key_hash;
    JzonValue* value;
};

struct JzonParseResult {
    bool success;
    JzonValue* output;
};

JzonParseResult jzon_parse(const char* input);
void jzon_free(JzonValue* value);
JzonValue* jzon_get(JzonValue* object, const char* key);
