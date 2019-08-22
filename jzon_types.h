#pragma once

fwd_struct(JzonKeyValuePair);

typedef struct JzonValue
{
    bool is_string : 1;
    bool is_int : 1;
    bool is_float : 1;
    bool is_table : 1;
    bool is_array : 1;
    bool is_bool : 1;
    bool is_null : 1;
    u64 size;

    union
    {
        char* string_val;
        i64 int_val;
        bool bool_val;
        f32 float_val;
        JzonKeyValuePair* table_val;
        struct JzonValue* array_val;
    };
} JzonValue;

typedef struct JzonKeyValuePair {
    char* key;
    hash64 key_hash;
    JzonValue val;
} JzonKeyValuePair;

typedef struct JzonParseResult {
    bool ok;
    JzonValue output;
} JzonParseResult;
