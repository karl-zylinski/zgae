#include "shader.h"
#include "jzon.h"
#include "file.h"
#include "str.h"
#include "array.h"
#include "renderer.h"
#include "memory.h"
#include "debug.h"
#include <stdio.h>

static shader_type_t type_str_to_enum(const char* str)
{
    if (str_eql(str, "vertex"))
        return SHADER_TYPE_VERTEX;

    if (str_eql(str, "fragment"))
        return SHADER_TYPE_FRAGMENT;

    return SHADER_TYPE_INVALID;
}

static shader_data_type_t data_type_str_to_enum(const char* str)
{
    if (str_eql(str, "mat4"))
        return SHADER_DATA_TYPE_MAT4;

    if (str_eql(str, "vec2"))
        return SHADER_DATA_TYPE_VEC2;

    if (str_eql(str, "vec3"))
        return SHADER_DATA_TYPE_VEC3;

    if (str_eql(str, "vec4"))
        return SHADER_DATA_TYPE_VEC4;

    return SHADER_DATA_TYPE_INVALID;
}

static shader_constant_buffer_auto_value_t cb_autoval_str_to_enum(const char* str)
{
    if (str_eql(str, "mat_model_view_projection"))
        return SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL_VIEW_PROJECTION;

    if (str_eql(str, "mat_model"))
        return SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL;

    if (str_eql(str, "mat_projection"))
        return SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_PROJECTION;

    if (str_eql(str, "mat_view_projection"))
        return SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_VIEW_PROJECTION;

    return SHADER_CONSTANT_BUFFER_AUTO_VALUE_NONE;
}

static shader_input_layout_value_t il_val_str_to_enum(const char* str)
{
    if (str_eql(str, "vertex_position"))
        return SHADER_INPUT_LAYOUT_VALUE_VERTEX_POSITION;

    if (str_eql(str, "vertex_normal"))
        return SHADER_INPUT_LAYOUT_VALUE_VERTEX_NORMAL;

    if (str_eql(str, "vertex_texcoord"))
        return SHADER_INPUT_LAYOUT_VALUE_VERTEX_TEXCOORD;

    if (str_eql(str, "vertex_color"))
        return SHADER_INPUT_LAYOUT_VALUE_VERTEX_COLOR;

    return SHADER_INPUT_LAYOUT_VALUE_INVALID;
}

renderer_resource_handle_t shader_load(renderer_state_t* rs, const char* filename)
{
    info("Loading shader from %s", filename);
    #define format_check(cond, msg, ...) (check(cond, "When parsing shader %s: %s", filename, msg, ##__VA_ARGS__))
    shader_intermediate_t si = {};
    file_load_result_t shader_flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
    format_check(shader_flr.ok, "File missing");
    jzon_parse_result_t jpr = jzon_parse(shader_flr.data);
    format_check(jpr.ok && jpr.output.is_table, "Malformed shader");
    memf(shader_flr.data);

    const jzon_value_t* jz_cb_tbl = jzon_get(&jpr.output, "constant_buffer");

    if (jz_cb_tbl)
    {
        format_check(jz_cb_tbl->is_table, "constant_buffer must be a table");
        const jzon_value_t* jz_cb_binding = jzon_get(jz_cb_tbl, "binding");
        format_check(jz_cb_binding && jz_cb_binding->is_int, "for constant_buffer: binding missing or isn't int");
        si.constant_buffer.binding = jz_cb_binding->int_val;

        const jzon_value_t* jz_cb_arr = jzon_get(jz_cb_tbl, "fields");
        format_check(jz_cb_arr && jz_cb_arr->is_array, "for constant_buffer: fields missing or isn't array");

        si.constant_buffer.items_num = (unsigned)jz_cb_arr->size;
        si.constant_buffer.items = mema_zero(sizeof(shader_constant_buffer_item_t) * si.constant_buffer.items_num);
        for (uint32_t i = 0; i < jz_cb_arr->size; ++i)
        {
            shader_constant_buffer_item_t* cbi = &si.constant_buffer.items[i];
            jzon_value_t* jz_cb_item = jz_cb_arr->array_val + i;
            format_check(jz_cb_item->is_table, "for constant_buffer.fields[%d]: field isn't table", i);

            const jzon_value_t* jz_cb_item_name = jzon_get(jz_cb_item, "name");
            format_check(jz_cb_item_name && jz_cb_item_name->is_string, "for constant_buffer.fields[%d]: name missing or isn't string", i);
            cbi->name = jz_cb_item_name->string_val;

            const jzon_value_t* jz_cb_item_type = jzon_get(jz_cb_item, "type");
            format_check(jz_cb_item_type && jz_cb_item_type->is_string, "for constant_buffer.fields[%d]: type missing or isn't string", i);
            shader_data_type_t sdt = data_type_str_to_enum(jz_cb_item_type->string_val);
            format_check(sdt, "for constant_buffer.fields[%d]: type isn't an allowed value", i);
            cbi->type = sdt;

            const jzon_value_t* jz_cb_item_autoval = jzon_get(jz_cb_item, "value");
            
            if (jz_cb_item_autoval && jz_cb_item_autoval->is_string)
            {
                shader_constant_buffer_auto_value_t auto_val = cb_autoval_str_to_enum(jz_cb_item_autoval->string_val);
                cbi->auto_value = auto_val;
            }
        }
    }

    const jzon_value_t* jz_il_arr = jzon_get(&jpr.output, "input_layout");

    if (jz_il_arr)
    {
        format_check(jz_il_arr->is_array, "input_layout must be array");
        si.input_layout_num = (unsigned)jz_il_arr->size;
        si.input_layout = mema_zero(sizeof(shader_input_layout_item_t) * si.input_layout_num);
        for (uint32_t i = 0; i < jz_il_arr->size; ++i)
        {
            shader_input_layout_item_t* ili = &si.input_layout[i];
            const jzon_value_t* jz_il_item = jz_il_arr->array_val + i;
            format_check(jz_il_item->is_table, "for input_layout[%d]: not a table");

            const jzon_value_t* jz_il_item_name = jzon_get(jz_il_item, "name");
            format_check(jz_il_item_name && jz_il_item_name->is_string, "for input_layout[%d]: name missing or not a string");
            ili->name = jz_il_item_name->string_val;

            const jzon_value_t* jz_il_item_type = jzon_get(jz_il_item, "type");
            format_check(jz_il_item_type && jz_il_item_type->is_string, "for input_layout[%d]: type missing or not a string");
            shader_data_type_t sdt = data_type_str_to_enum(jz_il_item_type->string_val);
            format_check(sdt, "for input_layout[%d]: type isn't an allowed value");
            ili->type = sdt;

            const jzon_value_t* jz_il_item_val = jzon_get(jz_il_item, "value");
            format_check(jz_il_item_val && jz_il_item_val->is_string, "for input_layout[%d]: value missing or not a string");
            shader_input_layout_value_t val = il_val_str_to_enum(jz_il_item_val->string_val);
            format_check(val, "for input_layout[%d]: value isn't an allowed value");
            ili->value = val;
        }
    }
    
    const jzon_value_t* jz_type = jzon_get(&jpr.output, "type");
    format_check(jz_type && jz_type->is_string, "type not a string or missing");
    shader_type_t st = type_str_to_enum(jz_type->string_val);
    format_check(st, "type isn't an allowed value");
    si.type = st;

    const jzon_value_t* jz_source = jzon_get(&jpr.output, "source");
    format_check(jz_source && jz_source->is_string, "source missing or not a string");

    file_load_result_t source_flr = file_load(jz_source->string_val, FILE_LOAD_MODE_DEFAULT);
    format_check(source_flr.ok, "failed opening shader source %s", jz_source->string_val);
    si.source = source_flr.data;
    si.source_size = source_flr.data_size;
    renderer_resource_handle_t rr = renderer_load_shader(rs, &si);

    memf(si.source);
    memf(si.constant_buffer.items);
    memf(si.input_layout);
    jzon_free(&jpr.output);

    return rr;
}

uint32_t shader_data_type_size(shader_data_type_t t)
{
    switch (t)
    {
        case SHADER_DATA_TYPE_MAT4: return 64;
        case SHADER_DATA_TYPE_VEC2: return 8;
        case SHADER_DATA_TYPE_VEC3: return 12;
        case SHADER_DATA_TYPE_VEC4: return 16;
        case SHADER_DATA_TYPE_INVALID: break;
    }

    error("Trying to get size of invalid shader_data_type_t");
    return 0;
}