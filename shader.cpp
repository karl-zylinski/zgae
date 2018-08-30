#include "shader.h"
#include "jzon.h"
#include "file.h"
#include "shader_intermediate.h"
#include "string_helpers.h"
#include "array.h"
#include "renderer.h"

static ShaderDataType type_str_to_enum(const char* str)
{
    if (str_eql(str, "mat4"))
        return ShaderDataType::Mat4;

    if (str_eql(str, "vec2"))
        return ShaderDataType::Vec2;

    if (str_eql(str, "vec3"))
        return ShaderDataType::Vec3;

    if (str_eql(str, "vec4"))
        return ShaderDataType::Vec4;

    return ShaderDataType::Invalid;
}

static ShaderConstantBufferAutoValue cb_autoval_str_to_enum(const char* str)
{
    if (str_eql(str, "mat_mvp"))
        return ShaderConstantBufferAutoValue::MatMVP;

    if (str_eql(str, "mat_model"))
        return ShaderConstantBufferAutoValue::MatModel;

    if (str_eql(str, "mat_projection"))
        return ShaderConstantBufferAutoValue::MatProjection;

    return ShaderConstantBufferAutoValue::None;
}

static ShaderInputLayoutValue il_val_str_to_enum(const char* str)
{
    if (str_eql(str, "vertex_position"))
        return ShaderInputLayoutValue::VertexPosition;

    if (str_eql(str, "vertex_normal"))
        return ShaderInputLayoutValue::VertexNormal;

    if (str_eql(str, "vertex_texcoord"))
        return ShaderInputLayoutValue::VertexTexCoord;

    if (str_eql(str, "vertex_color"))
        return ShaderInputLayoutValue::VertexColor;

    return ShaderInputLayoutValue::Invalid;
}

RRHandle shader_load(Renderer* renderer, const char* filename)
{
    ShaderIntermediate si = {};
    #define ensure(expr) if (!(expr)) return {InvalidHandle}
    LoadedFile shader_file = file_load(filename, FileEnding::Zero);
    ensure(shader_file.valid);
    JzonParseResult jpr = jzon_parse((const char*)shader_file.file.data);
    zfree(shader_file.file.data);
    ensure(jpr.valid && jpr.output.is_table);

    JzonValue* jz_cb_arr = jzon_get(&jpr.output, "constant_buffer");
    ensure(jz_cb_arr != nullptr && jz_cb_arr->is_array);
    si.constant_buffer_size = (unsigned)jz_cb_arr->size;
    si.constant_buffer = (ShaderConstantBufferItem*)zalloc_zero(sizeof(ShaderConstantBufferItem) * si.constant_buffer_size);
    for (size_t i = 0; i < jz_cb_arr->size; ++i)
    {
        ShaderConstantBufferItem& cbi = si.constant_buffer[i];
        JzonValue* jz_cb_item = jz_cb_arr->array_val + i;
        ensure(jz_cb_item->is_table);

        JzonValue* jz_cb_item_name = jzon_get(jz_cb_item, "name");
        ensure(jz_cb_item_name != nullptr && jz_cb_item_name->is_string);
        cbi.name = jz_cb_item_name->string_val;

        JzonValue* jz_cb_item_type = jzon_get(jz_cb_item, "type");
        ensure(jz_cb_item_type != nullptr && jz_cb_item_type->is_string);
        ShaderDataType sdt = type_str_to_enum(jz_cb_item_type->string_val);
        ensure(sdt != ShaderDataType::Invalid);
        cbi.type = sdt;

        JzonValue* jz_cb_item_autoval = jzon_get(jz_cb_item, "value");
        
        if (jz_cb_item_autoval != nullptr && jz_cb_item_autoval->is_string)
        {
            ShaderConstantBufferAutoValue auto_val = cb_autoval_str_to_enum(jz_cb_item_autoval->string_val);
            cbi.auto_value = auto_val;
        }
    }

    JzonValue* jz_il_arr = jzon_get(&jpr.output, "input_layout");
    ensure(jz_il_arr != nullptr && jz_il_arr->is_array);
    si.input_layout_size = (unsigned)jz_il_arr->size;
    si.input_layout = (ShaderInputLayoutItem*)zalloc_zero(sizeof(ShaderInputLayoutItem) * si.input_layout_size);
    for (size_t i = 0; i < jz_il_arr->size; ++i)
    {
        ShaderInputLayoutItem& ili = si.input_layout[i];
        JzonValue* jz_il_item = jz_il_arr->array_val + i;
        ensure(jz_il_item->is_table);

        JzonValue* jz_il_item_name = jzon_get(jz_il_item, "name");
        ensure(jz_il_item_name != nullptr && jz_il_item_name->is_string);
        ili.name = jz_il_item_name->string_val;

        JzonValue* jz_il_item_type = jzon_get(jz_il_item, "type");
        ensure(jz_il_item_type != nullptr && jz_il_item_type->is_string);
        ShaderDataType sdt = type_str_to_enum(jz_il_item_type->string_val);
        ensure(sdt != ShaderDataType::Invalid);
        ili.type = sdt;

        JzonValue* jz_il_item_val = jzon_get(jz_il_item, "value");
        ensure(jz_il_item_val != nullptr && jz_il_item_val->is_string);
        ShaderInputLayoutValue val = il_val_str_to_enum(jz_il_item_val->string_val);
        ili.value = val;
    }

    JzonValue* jz_source = jzon_get(&jpr.output, "source_hlsl");
    ensure(jz_source != nullptr && jz_source->is_string);
    LoadedFile lf_source = file_load(jz_source->string_val, FileEnding::None);
    ensure(lf_source.valid);
    si.source = (char*)lf_source.file.data;
    si.source_size = lf_source.file.size;

    RRHandle h = renderer->load_shader(si);

    zfree(si.source);
    zfree(si.constant_buffer);
    zfree(si.input_layout);
    jzon_free(&jpr.output);

    return h;
}
