#include "pipeline_resource.h"
#include "shader_resource.h"
#include "debug.h"
#include "file.h"
#include "jzon.h"
#include "memory.h"
#include "renderer.h"
#include "str.h"

static ShaderDataType shader_data_type_str_to_enum(const char* str)
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

static ConstantBufferAutoValue cb_autoval_str_to_enum(const char* str)
{
    if (str_eql(str, "mat_model_view_projection"))
        return CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL_VIEW_PROJECTION;

    if (str_eql(str, "mat_model"))
        return CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL;

    if (str_eql(str, "mat_projection"))
        return CONSTANT_BUFFER_AUTO_VALUE_MAT_PROJECTION;

    if (str_eql(str, "mat_view_projection"))
        return CONSTANT_BUFFER_AUTO_VALUE_MAT_VIEW_PROJECTION;

    return CONSTANT_BUFFER_AUTO_VALUE_NONE;
}

static VertexInputValue il_val_str_to_enum(const char* str)
{
    if (str_eql(str, "position"))
        return VERTEX_INPUT_VALUE_POSITION;

    if (str_eql(str, "normal"))
        return VERTEX_INPUT_VALUE_NORMAL;

    if (str_eql(str, "texcoord"))
        return VERTEX_INPUT_VALUE_TEXCOORD;

    if (str_eql(str, "color"))
        return VERTEX_INPUT_VALUE_COLOR;

    return VERTEX_INPUT_VALUE_INVALID;
}


PipelineResource pipeline_resource_load(const char* filename)
{
    info("Loading pipeline from %s", filename);
    PipelineResource pr = {};
    #define ensure(expr) if (!(expr)) error("Error in pipeline resource load");
    FileLoadResult flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
    ensure(flr.ok);
    JzonParseResult jpr = jzon_parse(flr.data);
    ensure(jpr.ok && jpr.output.is_table);
    memf(flr.data);

    const JzonValue* jz_shader_stages = jzon_get(&jpr.output, "shader_stages");
    ensure(jz_shader_stages && jz_shader_stages->is_array);
    pr.shader_stages_num = jz_shader_stages->size;
    pr.shader_stages = mema_zero(sizeof(ShaderResource) * pr.shader_stages_num);
    
    for (u32 shdr_idx = 0; shdr_idx < jz_shader_stages->size; ++shdr_idx)
    {
        const JzonValue* jz_shader_stage = jz_shader_stages->array_val + shdr_idx;
        ensure(jz_shader_stage->is_string);
        pr.shader_stages[shdr_idx] = shader_resource_load(jz_shader_stage->string_val);
    }

    const JzonValue* jz_constant_buffers = jzon_get(&jpr.output, "constant_buffers");

    if (jz_constant_buffers)
    {
        ensure(jz_constant_buffers->is_array);
        pr.constant_buffers_num = jz_constant_buffers->num;
        pr.constant_buffers = mema_zero(sizeof(ConstantBuffer) * pr.constant_buffers_num);

        for (u32 cb_idx = 0; cb_idx < pr.constant_buffers_num; ++cb_idx)
        {
            const JzonValue* jz_constant_buffer = jz_constant_buffers->array_val + cb_idx;
            ensure(jz_constant_buffer->is_table);

            const JzonValue* jz_binding = jzon_get(jz_constant_buffer, "binding");
            ensure(jz_binding && jz_binding->is_int);
            pr.constant_buffers[cb_idx].binding = jz_binding->int_val;

            const JzonValue* jz_fields = jzon_get(jz_constant_buffer, "fields");
            ensure(jz_fields && jz_fields->is_array);

            pr.constant_buffers[cb_idx].fields_num = (unsigned)jz_fields->size;
            pr.constant_buffers[cb_idx].fields = mema_zero(sizeof(ConstantBuffer) * pr.constant_buffers[cb_idx].fields_num);
            for (u32 i = 0; i < jz_fields->size; ++i)
            {
                ConstantBufferField* cbf = pr.constant_buffers[cb_idx].fields + i;
                const JzonValue* jz_cbf = jz_fields->array_val + i;
                ensure(jz_cbf->is_table);

                const JzonValue* jz_name = jzon_get(jz_cbf, "name");
                ensure(jz_name && jz_name->is_string);
                cbf->name = jz_name->string_val;

                const JzonValue* jz_type = jzon_get(jz_cbf, "type");
                ensure(jz_type && jz_type->is_string)
                ShaderDataType sdt = shader_data_type_str_to_enum(jz_type->string_val);
                ensure(sdt);
                cbf->type = sdt;

                const JzonValue* jz_cbf_autoval = jzon_get(jz_cbf, "value");
                
                if (jz_cbf_autoval && jz_cbf_autoval->is_string)
                {
                    ConstantBufferAutoValue auto_val = cb_autoval_str_to_enum(jz_cbf_autoval->string_val);
                    cbf->auto_value = auto_val;
                }
            }
        }
    }    

    const JzonValue* jz_vertex_input = jzon_get(&jpr.output, "vertex_input");

    if (jz_vertex_input)
    {
        ensure(jz_vertex_input && jz_vertex_input->is_array);
        pr.vertex_input_num = (u32)jz_vertex_input->size;
        pr.vertex_input = mema_zero(sizeof(VertexInputField) * pr.vertex_input_num);
        for (u32 i = 0; i < jz_vertex_input->size; ++i)
        {
            VertexInputField* vif = &pr.vertex_input[i];
            const JzonValue* jz_vif = jz_vertex_input->array_val + i;
            ensure(jz_vif && jz_vif->is_table);

            const JzonValue* jz_name = jzon_get(jz_vif, "name");
            ensure(jz_name && jz_name->is_string);
            vif->name = jz_name->string_val;

            const JzonValue* jz_type = jzon_get(jz_vif, "type");
            ensure(jz_type && jz_type->is_string);
            ShaderDataType sdt = shader_data_type_str_to_enum(jz_type->string_val);
            ensure(sdt);
            vif->type = sdt;

            const JzonValue* jz_vif_val = jzon_get(jz_vif, "value");
            ensure(jz_vif_val && jz_vif_val->is_string);
            VertexInputValue val = il_val_str_to_enum(jz_vif_val->string_val);
            ensure(val);
            vif->value = val;
        }
    }

    jzon_free(&jpr.output);
    return pr;
}