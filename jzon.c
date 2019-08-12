#include "jzon.h"
#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "str.h"

static void next(const char** input)
{
    ++*input;
}

static char current(const char** input)
{
    return **input;
}

static int is_str(const char* input, const char* str)
{
    size_t len = strlen(str);

    for (unsigned i = 0; i < len; ++i)
    {
        if (input[i] == 0)
            return 0;

        if (input[i] != str[i])
            return 0;
    }

    return 1;
}

static int is_multiline_string_quotes(const char* str)
{
    return is_str(str, "\"\"\"");
}

static size_t find_table_pair_insertion_index(jzon_key_value_pair_t* table, int64_t key_hash)
{
    if (array_size(table) == 0)
        return 0;

    for (unsigned i = 0; i < array_size(table); ++i)
    {
        if (table[i].key_hash > key_hash)
            return i;
    }

    return array_size(table);
}

static int is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static void skip_whitespace(const char** input)
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

static char* parse_multiline_string(const char** input)
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

static char* parse_string_internal(const char** input)
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

static char* parse_keyname(const char** input)
{
    if (current(input) == '"')
        return parse_string_internal(input);

    char* start = (char*)*input;

    while (current(input))
    {
        const char* cur_wo_whitespace = *input;
        if (is_whitespace(current(input)))
            skip_whitespace(input);

        if (current(input) == '=')
            return str_copy_s(start, (unsigned)(cur_wo_whitespace - start));

        next(input);
    }

    return NULL;
}

static int parse_string(const char** input, jzon_value_t* output)
{
    char* str = parse_string_internal(input);

    if (!str)
        return 0;

    output->is_string = 1;
    output->string_val = str;
    return 1;
}

static int parse_value(const char** input, jzon_value_t* output);

static int parse_array(const char** input, jzon_value_t* output)
{   
    if (current(input) != '[')
        return 0;
    
    output->is_array = 1;
    next(input);
    skip_whitespace(input);

    // Empty array.
    if (current(input) == ']')
    {
        next(input);
        output->size = 0; 
        return 1;
    }

    jzon_value_t* array = NULL;

    while (current(input))
    {
        skip_whitespace(input);
        jzon_value_t value = {};

        if (!parse_value(input, &value))
            return 0;

        array_push(array, value);
        skip_whitespace(input);

        if (current(input) == ']')
        {
            next(input);
            break;
        }
    }
    
    output->size = array_size(array);
    output->array_val = (jzon_value_t*)array_copy_data(array);
    return 1;
}

static int parse_table(const char** input, jzon_value_t* output, int root_table)
{
    if (current(input) == '{')
        next(input);
    else if (!root_table)
        return 0;

    output->is_table = 1;
    skip_whitespace(input);

    // Empty object.
    if (current(input) == '}')
    {
        output->size = 0;
        return 1;
    }

    jzon_key_value_pair_t* table = NULL;

    while (current(input))
    {
        skip_whitespace(input);
        char* key = parse_keyname(input);
        skip_whitespace(input);

        if (!key || current(input) != '=')
            return 0;

        next(input);
        jzon_value_t value = {};

        if (!parse_value(input, &value))
            return 0;

        jzon_key_value_pair_t pair = {};
        pair.key = key;
        pair.key_hash = str_hash(key);
        pair.val = value;
        array_insert(table, pair, find_table_pair_insertion_index(table, pair.key_hash));
        skip_whitespace(input);

        if (current(input) == '}')
        {
            next(input);
            break;
        }
    }

    output->size = array_size(table);
    output->table_val = (jzon_key_value_pair_t*)array_copy_data(table);
    return 1;
}

static int parse_number(const char** input, jzon_value_t* output)
{
    int is_float = 0;
    char* start = (char*)*input;

    if (current(input) == '-')
        next(input);

    while (current(input) >= '0' && current(input) <= '9')
        next(input);

    if (current(input) == '.')
    {
        is_float = 1;
        next(input);

        while (current(input) >= '0' && current(input) <= '9')
            next(input);
    }

    if (current(input) == 'e' || current(input) == 'E')
    {
        is_float = 1;
        next(input);

        if (current(input) == '-' || current(input) == '+')
            next(input);

        while (current(input) >= '0' && current(input) <= '9')
            next(input);
    }

    if (is_float)
    {
        output->is_float = 1;
        output->float_val = (float)strtod(start, NULL);
    }
    else
    {
        output->is_int = 1;
        output->int_val = (int32_t)strtol(start, NULL, 10);
    }

    return 1;
}

static int parse_true(const char** input, jzon_value_t* output)
{
    if (is_str(*input, "true"))
    {
        output->is_bool = 1;
        output->bool_val = 1;
        return 1;
    }

    return 0;
}

static int parse_false(const char** input, jzon_value_t* output)
{
    if (is_str(*input, "false"))
    {
        output->is_bool = 1;
        output->bool_val = 0;
        *input += 5;
        return 1;
    }

    return 0;
}

static int parse_null(const char** input, jzon_value_t* output)
{
    if (is_str(*input, "null"))
    {
        output->is_null = 1;
        *input += 4;
        return 1;
    }

    return 0;
}

static int parse_value(const char** input, jzon_value_t* output)
{
    skip_whitespace(input);
    char ch = current(input);

    switch (ch)
    {
        case '{': return parse_table(input, output, 0);
        case '[': return parse_array(input, output);
        case '"': return parse_string(input, output);
        case '-': return parse_number(input, output);
        case 'f': return parse_false(input, output);
        case 't': return parse_true(input, output);
        case 'n': return parse_null(input, output);
        default: return ch >= '0' && ch <= '9' ? parse_number(input, output) : 0;
    }
}

int jzon_parse(const char* input, jzon_value_t* output)
{
    memset(output, 0, sizeof(jzon_value_t));
    skip_whitespace(&input);
    return parse_table(&input, output, 1);
}

void jzon_free(jzon_value_t* val)
{
    if (val->is_table)
    {
        for (uint32_t i = 0; i < val->size; ++i)
        {
            memf(val->table_val[i].key);
            jzon_free(&val->table_val[i].val);
        }

        memf(val->table_val);
    }
    else if (val->is_array)
    {
        for (uint32_t i = 0; i < val->size; ++i)
            jzon_free(&val->array_val[i]);

        memf(val->array_val);
    }
    else if (val->is_string)
    {
        memf(val->string_val);
    }
}

jzon_value_t* jzon_get(jzon_value_t* table, const char* key)
{
    if (!table->is_table)
        return NULL;

    if (table->size == 0)
        return NULL;
    
    int64_t key_hash = str_hash(key);

    size_t first = 0;
    size_t last = table->size - 1;
    size_t middle = (first + last) / 2;

    while (first <= last)
    {
        if (table->table_val[middle].key_hash < key_hash)
            first = middle + 1;
        else if (table->table_val[middle].key_hash == key_hash)
            return &table->table_val[middle].val;
        else
            last = middle - 1;

        middle = (first + last) / 2;
    }

    return NULL;
}
