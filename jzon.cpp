#include "jzon.h"
#include <string.h>
#include "dynamic_array.h"
#include "memory.h"
#include "str.h"
#include <stdlib.h>

static void next(char** input)
{
    ++*input;
}

static char current(char** input)
{
    return **input;
}

static bool is_str(char* input, char* str)
{
    u32 len = strlen(str);

    for (unsigned i = 0; i < len; ++i)
    {
        if (input[i] == 0)
            return false;

        if (input[i] != str[i])
            return false;
    }

    return true;
}

static bool is_multiline_string_quotes(char* str)
{
    return is_str(str, "\"\"\"");
}

static u64 find_table_pair_insertion_index(JzonKeyValuePair* table, i64 key_hash)
{
    u32 n = da_num(table);

    if (n == 0)
        return 0;

    for (u32 i = 0; i < n; ++i)
    {
        if (table[i].key_hash > key_hash)
            return i;
    }

    return n;
}

static bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static void skip_whitespace(char** input)
{
    while (current(input))
    {
        while (current(input) && (is_whitespace(current(input)) || current(input) == ','))
            next(input);
        
        // Skip comment.
        if (current(input) == '#')
        {
            while (current(input) && current(input) != '\n')
                next(input);
        }
        else
            break;
    }
};

static char* parse_multiline_string(char** input)
{
    if (!is_multiline_string_quotes(*input))
        return NULL;
    
    *input += 3;
    char* start = (char*)*input;
    char* result = (char*)mema_zero(1);

    while (current(input))
    {
        if (current(input) == '\n' || current(input) == '\r')
        {
            unsigned result_len = (unsigned)strlen(result);
            unsigned line_len = (unsigned)(*input - start);

            if (result_len > 0) {
                str_app_s(result, "\n", 1);
                ++result_len;
            }

            skip_whitespace(input);

            if (line_len != 0)
            {
                str_app_s(result, start, line_len);
            }

            start = (char*)*input;
        }

        if (is_multiline_string_quotes(*input))
        {
            str_app_s(result, start, (unsigned)(*input - start));
            *input += 3;
            return result;
        }

        next(input);
    }

    memf(result);
    return NULL;
}

static char* parse_string_internal(char** input)
{
    if (current(input) != '"')
        return NULL;

    if (is_multiline_string_quotes(*input))
        return parse_multiline_string(input);

    next(input);
    char* start = (char*)*input;

    while (current(input))
    {
        if (current(input) == '"')
        {
            char* end = (char*)*input;
            next(input);
            return str_copy_s(start, (unsigned)(end - start));
            break;
        }

        next(input);
    }

    return NULL;
}

static char* parse_keyname(char** input)
{
    if (current(input) == '"')
        return parse_string_internal(input);

    char* start = (char*)*input;

    while (current(input))
    {
        char* cur_wo_whitespace = *input;
        if (is_whitespace(current(input)))
            skip_whitespace(input);

        if (current(input) == '=')
            return str_copy_s(start, (unsigned)(cur_wo_whitespace - start));

        next(input);
    }

    return NULL;
}

static bool parse_string(char** input, JzonValue* output)
{
    char* str = parse_string_internal(input);

    if (!str)
        return false;

    output->is_string = true;
    output->string_val = str;
    return true;
}

static bool parse_value(char** input, JzonValue* output);

static bool parse_array(char** input, JzonValue* output)
{   
    if (current(input) != '[')
        return false;
    
    output->is_array = true;
    next(input);
    skip_whitespace(input);

    // Empty array.
    if (current(input) == ']')
    {
        next(input);
        output->size = 0; 
        return true;
    }

    JzonValue* array = NULL;

    while (current(input))
    {
        skip_whitespace(input);
        JzonValue value = {};

        if (!parse_value(input, &value))
            return false;

        da_push(array, value);
        skip_whitespace(input);

        if (current(input) == ']')
        {
            next(input);
            break;
        }
    }
    
    output->size = da_num(array);
    output->array_val = (JzonValue*)da_copy_data(array);
    da_free(array);
    return true;
}

static bool parse_table(char** input, JzonValue* output, bool root_table)
{
    if (current(input) == '{')
        next(input);
    else if (!root_table)
        return false;

    output->is_table = true;
    skip_whitespace(input);

    // Empty object.
    if (current(input) == '}')
    {
        output->size = 0;
        return true;
    }

    JzonKeyValuePair* table = NULL;

    while (current(input))
    {
        skip_whitespace(input);
        char* key = parse_keyname(input);
        skip_whitespace(input);

        if (!key || current(input) != '=')
            return false;

        next(input);
        JzonValue value = {};

        if (!parse_value(input, &value))
            return false;

        JzonKeyValuePair pair = {};
        pair.key = key;
        pair.key_hash = str_hash(key);
        pair.val = value;
        da_insert(table, pair, find_table_pair_insertion_index(table, pair.key_hash));
        skip_whitespace(input);

        if (current(input) == '}')
        {
            next(input);
            break;
        }
    }

    output->size = da_num(table);
    output->table_val = (JzonKeyValuePair*)da_copy_data(table);
    da_free(table);
    return true;
}

static bool parse_number(char** input, JzonValue* output)
{
    bool is_float = false;
    char* start = (char*)*input;

    if (current(input) == '-')
        next(input);

    while (current(input) >= '0' && current(input) <= '9')
        next(input);

    if (current(input) == '.')
    {
        is_float = true;
        next(input);

        while (current(input) >= '0' && current(input) <= '9')
            next(input);
    }

    if (current(input) == 'e' || current(input) == 'E')
    {
        is_float = true;
        next(input);

        if (current(input) == '-' || current(input) == '+')
            next(input);

        while (current(input) >= '0' && current(input) <= '9')
            next(input);
    }

    if (is_float)
    {
        output->is_float = true;
        output->float_val = (f32)strtod(start, NULL);
    }
    else
    {
        output->is_int = true;
        output->int_val = (i32)strtol(start, NULL, 10);
    }

    return true;
}

static bool parse_true(char** input, JzonValue* output)
{
    if (is_str(*input, "true"))
    {
        output->is_bool = true;
        output->bool_val = true;
        *input += 4;
        return true;
    }

    return false;
}

static bool parse_false(char** input, JzonValue* output)
{
    if (is_str(*input, "false"))
    {
        output->is_bool = true;
        output->bool_val = false;
        *input += 5;
        return true;
    }

    return false;
}

static bool parse_null(char** input, JzonValue* output)
{
    if (is_str(*input, "null"))
    {
        output->is_null = true;
        *input += 4;
        return true;
    }

    return false;
}

static bool parse_value(char** input, JzonValue* output)
{
    skip_whitespace(input);
    char ch = current(input);

    switch (ch)
    {
        case '{': return parse_table(input, output, false);
        case '[': return parse_array(input, output);
        case '"': return parse_string(input, output);
        case '-': return parse_number(input, output);
        case 'f': return parse_false(input, output);
        case 't': return parse_true(input, output);
        case 'n': return parse_null(input, output);
        default: return ch >= '0' && ch <= '9' ? parse_number(input, output) : 0;
    }
}

JzonParseResult jzon_parse(char* input)
{
    JzonValue output = {};
    skip_whitespace(&input);
    bool ok = parse_table(&input, &output, true);

    JzonParseResult pr = {
        .ok = ok,
        .output = output
    };

    return pr;
}

void jzon_free(JzonValue* val)
{
    if (val->is_table)
    {
        for (u32 i = 0; i < val->size; ++i)
        {
            memf(val->table_val[i].key);
            jzon_free(&val->table_val[i].val);
        }

        memf(val->table_val);
    }
    else if (val->is_array)
    {
        for (u32 i = 0; i < val->size; ++i)
            jzon_free(&val->array_val[i]);

        memf(val->array_val);
    }
    else if (val->is_string)
    {
        memf(val->string_val);
    }
}

const JzonValue* jzon_get(const JzonValue& table, char* key)
{
    if (!table.is_table)
        return NULL;

    if (table.size == 0)
        return NULL;
    
    i64 key_hash = str_hash(key);

    u32 first = 0;
    u32 last = table.size - 1;
    u32 middle = (first + last) / 2;

    while (first <= last)
    {
        if (table.table_val[middle].key_hash < key_hash)
            first = middle + 1;
        else if (table.table_val[middle].key_hash == key_hash)
            return &table.table_val[middle].val;
        else
        {
            if (middle == 0)
                break;

            last = middle - 1;
        }

        middle = (first + last) / 2;
    }

    return NULL;
}