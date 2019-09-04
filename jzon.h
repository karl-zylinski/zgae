#pragma once

fwd_struct(JzonKeyValuePair);

struct JzonValue
{
    bool is_string : 1;
    bool is_int : 1;
    bool is_float : 1;
    bool is_table : 1;
    bool is_array : 1;
    bool is_bool : 1;
    bool is_null : 1;

    union
    {
        u32 size;
        u32 num;
    };

    union
    {
        char* string_val;
        i64 int_val;
        bool bool_val;
        f32 float_val;
        JzonKeyValuePair* table_val;
        JzonValue* array_val;
    };
};

struct JzonKeyValuePair
{
    char* key;
    hash64 key_hash;
    JzonValue val;
};

struct JzonParseResult
{
    bool ok;
    JzonValue output;
};

JzonParseResult jzon_parse(char* input);
void jzon_free(JzonValue* val);
const JzonValue* jzon_get(const JzonValue& table, char* key);