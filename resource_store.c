#include "resource_store.h"
#include "resource_types.h"
#include "handle_pool.h"
#include "path.h"
#include "str.h"
#include "debug.h"
#include "array.h"
#include "jzon.h"
#include "file.h"
#include "memory.h"
#include "renderer.h"

static HandlePool* g_hp = NULL;
static RendererState* g_rs = NULL;
static Resource* da_resources = NULL;

typedef struct ResourceFilenameMapping
{
    hash64 name_hash;
    ResourceHandle handle;
} ResourceFilenameMapping;

ResourceFilenameMapping* g_mapping = NULL;

static size_t find_mapping_insertion_idx(hash64 name_hash)
{
    if (array_num(g_mapping) == 0)
        return 0;

    for (size_t i = 0; i < array_num(g_mapping); ++i)
    {
        if (g_mapping[i].name_hash > name_hash)
            return i;
    }

    return array_num(g_mapping);
}

static size_t mapping_get_idx(hash64 name_hash)
{
    if (array_num(g_mapping) == 0)
        return -1;

    size_t mz = array_num(g_mapping);
    size_t first = 0;
    size_t last = mz - 1;
    size_t middle = (first + last) / 2;

    while (first <= last)
    {
        if (g_mapping[middle].name_hash < name_hash)
            first = middle + 1;
        else if (g_mapping[middle].name_hash == name_hash)
            return middle;
        else
        {
            if (middle == 0)
                break;

            last = middle - 1;
        }

        middle = (first + last) / 2;
    }

    return -1;
}

static ResourceHandle mapping_get(hash64 name_hash)
{
    size_t idx = mapping_get_idx(name_hash);

    if (idx == (size_t)-1)
        return HANDLE_INVALID;

    return g_mapping[idx].handle;
}

static void mapping_remove(hash64 name_hash)
{
    size_t idx = mapping_get_idx(name_hash);

    if (idx == (size_t)-1)
        return;

    info("Implment me, leave holes? What do...");
}

static const char* resource_type_names[] = {
    "invalid",
    "shader",
    "pipeline"
};

static ResourceType resource_type_from_str(const char* str)
{
    i32 idx = str_eql_arr(str, resource_type_names, arrnum(resource_type_names));
    check(idx > 0 && idx < RESOURCE_TYPE_NUM, "Invalid resource type");
    return idx;
}

void resource_store_init(RendererState* rs)
{
    check(!g_hp && !g_rs, "resource_store_init probably run twice");

    g_hp = handle_pool_create(0, "ResourceHandle");
    g_rs = rs;

    for (ResourceType s = 1; s < RESOURCE_TYPE_NUM; ++s)
        handle_pool_set_subtype(g_hp, s, resource_type_names[s]);
}

static void resource_destroy_internal(const Resource* r)
{
    switch(r->type)
    {
        case RESOURCE_TYPE_SHADER: {
            memf(r->shader.source);
        } break;

        case RESOURCE_TYPE_PIPELINE: {
            const PipelineResource* pr = &r->pipeline;
            memf(pr->shader_stages);

            for (u32 i = 0; i < pr->constant_buffers_num; ++i)
            {
                for (u32 j = 0; j < pr->constant_buffers[i].fields_num; ++j)
                    memf(pr->constant_buffers[i].fields[j].name);

                memf(pr->constant_buffers[i].fields);
            }

            memf(pr->constant_buffers);

            for (u32 i = 0; i < pr->vertex_input_num; ++i)
                memf(pr->vertex_input[i].name);
            
            memf(pr->vertex_input);
            
        } break;

        default: error("Implement me!"); break;
    }

    handle_pool_return(g_hp, r->handle);
    mapping_remove(r->name_hash);
}

void resource_store_destroy()
{
    for (size_t i = 0; i < array_num(da_resources); ++i)
        resource_destroy_internal(da_resources + i);

    array_destroy(da_resources);
    array_destroy(g_mapping);
    handle_pool_destroy(g_hp);
}

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

static ShaderType shader_type_str_to_enum(const char* str)
{
    if (str_eql(str, "vertex"))
        return SHADER_TYPE_VERTEX;

    if (str_eql(str, "fragment"))
        return SHADER_TYPE_FRAGMENT;

    return SHADER_TYPE_INVALID;
}

ResourceHandle resource_load(const char* filename)
{
    hash64 name_hash = str_hash(filename);
    ResourceHandle existing = mapping_get(name_hash);

    if (existing != HANDLE_INVALID)
        return existing;

    const char* ext = path_ext(filename);
    ResourceType type = resource_type_from_str(ext);
    Resource r = { .type = type, .name_hash = name_hash };
    
    info("Loading resource %s", filename);

    switch(type)
    {
        case RESOURCE_TYPE_SHADER: {
            #define format_check(cond, msg, ...) (check(cond, "When parsing shader %s: %s", filename, msg, ##__VA_ARGS__))
            ShaderResource sr = {};
            FileLoadResult shader_flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
            format_check(shader_flr.ok, "File missing");
            JzonParseResult jpr = jzon_parse(shader_flr.data);
            format_check(jpr.ok && jpr.output.is_table, "Malformed shader");
            memf(shader_flr.data);
            
            const JzonValue* jz_type = jzon_get(&jpr.output, "type");
            format_check(jz_type && jz_type->is_string, "type not a string or missing");
            ShaderType st = shader_type_str_to_enum(jz_type->string_val);
            format_check(st, "type isn't an allowed value");
            sr.type = st;

            const JzonValue* jz_source = jzon_get(&jpr.output, "source");
            format_check(jz_source && jz_source->is_string, "source missing or not a string");

            FileLoadResult source_flr = file_load(jz_source->string_val, FILE_LOAD_MODE_DEFAULT);
            format_check(source_flr.ok, "failed opening shader source %s", jz_source->string_val);
            sr.source = mema_copy(source_flr.data, source_flr.data_size);
            sr.source_size = source_flr.data_size;
            memf(source_flr.data);
            jzon_free(&jpr.output);
            r.shader = sr;
        } break;

        case RESOURCE_TYPE_PIPELINE: {
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
                pr.shader_stages[shdr_idx] = resource_load(jz_shader_stage->string_val);
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
                        cbf->name = str_copy(jz_name->string_val);

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
                    vif->name = str_copy(jz_name->string_val);

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
                       
            r.pipeline = pr;
        } break;

        default: error("Implement me!"); break;
    }

    ResourceHandle h = handle_pool_borrow(g_hp, r.type);
    r.handle = h;
    array_fill_and_set(da_resources, handle_index(h), r);
    ResourceFilenameMapping rfm = {.handle = h, .name_hash = name_hash};
    array_insert(g_mapping, rfm, find_mapping_insertion_idx(name_hash));
    da_resources[handle_index(h)].rrh = renderer_create_renderer_resource(g_rs, h);
    return h;
}

const Resource* resource_lookup(ResourceHandle h)
{
    if (!handle_pool_is_valid(g_hp, h))
        return NULL;

    return da_resources + handle_index(h);
}

void resource_destroy(ResourceHandle h)
{
    resource_destroy_internal(resource_lookup(h));
}