#include "shader.h"
#include "jzon.h"
#include "file.h"
#include "str.h"
#include "array.h"
#include "renderer.h"
#include "memory.h"
#include "debug.h"

static shader_data_type_t type_str_to_enum(const char* str)
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

renderer_resource_t shader_load(renderer_state_t* rs, const char* filename)
{
    info("Loading shader from %s", filename);
    shader_intermediate_t si = {};
    #define ensure(expr) if (!(expr)) return HANDLE_INVALID
    char* fd;
    uint64_t fs;
    file_load_str(filename, &fd, &fs);
    ensure(fd);
    jzon_value_t parsed;
    int parse_res = jzon_parse(fd, &parsed);
    ensure(parse_res && parsed.is_table);
    memf(fd);

    jzon_value_t* jz_cb_arr = jzon_get(&parsed, "constant_buffer");
    ensure(jz_cb_arr && jz_cb_arr->is_array);
    si.constant_buffer_size = (unsigned)jz_cb_arr->size;
    si.constant_buffer = mema_zero(sizeof(shader_constant_buffer_item_t) * si.constant_buffer_size);
    for (uint32_t i = 0; i < jz_cb_arr->size; ++i)
    {
        shader_constant_buffer_item_t* cbi = &si.constant_buffer[i];
        jzon_value_t* jz_cb_item = jz_cb_arr->array_val + i;
        ensure(jz_cb_item->is_table);

        jzon_value_t* jz_cb_item_name = jzon_get(jz_cb_item, "name");
        ensure(jz_cb_item_name && jz_cb_item_name->is_string);
        cbi->name = jz_cb_item_name->string_val;

        jzon_value_t* jz_cb_item_type = jzon_get(jz_cb_item, "type");
        ensure(jz_cb_item_type && jz_cb_item_type->is_string);
        shader_data_type_t sdt = type_str_to_enum(jz_cb_item_type->string_val);
        ensure(sdt);
        cbi->type = sdt;

        jzon_value_t* jz_cb_item_autoval = jzon_get(jz_cb_item, "value");
        
        if (jz_cb_item_autoval && jz_cb_item_autoval->is_string)
        {
            shader_constant_buffer_auto_value_t auto_val = cb_autoval_str_to_enum(jz_cb_item_autoval->string_val);
            cbi->auto_value = auto_val;
        }
    }

    jzon_value_t* jz_il_arr = jzon_get(&parsed, "input_layout");
    ensure(jz_il_arr && jz_il_arr->is_array);
    si.input_layout_size = (unsigned)jz_il_arr->size;
    si.input_layout = mema_zero(sizeof(shader_input_layout_item_t) * si.input_layout_size);
    for (uint32_t i = 0; i < jz_il_arr->size; ++i)
    for (uint32_t i = 0; i < jz_il_arr->size; ++i)
    {
        shader_input_layout_item_t* ili = &si.input_layout[i];
        jzon_value_t* jz_il_item = jz_il_arr->array_val + i;
        ensure(jz_il_item->is_table);

        jzon_value_t* jz_il_item_name = jzon_get(jz_il_item, "name");
        ensure(jz_il_item_name && jz_il_item_name->is_string);
        ili->name = jz_il_item_name->string_val;

        jzon_value_t* jz_il_item_type = jzon_get(jz_il_item, "type");
        ensure(jz_il_item_type && jz_il_item_type->is_string);
        shader_data_type_t sdt = type_str_to_enum(jz_il_item_type->string_val);
        ensure(sdt);
        ili->type = sdt;

        jzon_value_t* jz_il_item_val = jzon_get(jz_il_item, "value");
        ensure(jz_il_item_val && jz_il_item_val->is_string);
        shader_input_layout_value_t val = il_val_str_to_enum(jz_il_item_val->string_val);
        ili->value = val;
    }

    jzon_value_t* jz_source = jzon_get(&parsed, "source_hlsl");
    ensure(jz_source && jz_source->is_string);

    int source_load_ok = file_load_str(jz_source->string_val, &si.source, &si.source_size);
    ensure(source_load_ok);

    (void)rs;
    //renderer_resource_t rr = renderer_load_shader(rs, si);

    memf(si.source);
    memf(si.constant_buffer);
    memf(si.input_layout);
    jzon_free(&parsed);

    return HANDLE_INVALID;
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