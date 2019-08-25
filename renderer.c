#include "renderer.h"
#include "window_types.h"
#include "renderer_backend.h"
#include "memory.h"
#include "handle_pool.h"
#include "array.h"
#include "debug.h"
#include "render_resource_types.h"
#include "render_resource.h"
#include "math.h"
#include "str.h"
#include "handle_hash_map.h"
#include "path.h"
#include "file.h"
#include "jzon.h"
#include "obj_loader.h"

typedef struct ShaderRenderResource
{
    char* source;
    u64 source_size;
    ShaderType type;
    RenderBackendShader* backend_state;
} ShaderRenderResource;

typedef struct PipelineRenderResource
{
    RenderResourceHandle* shader_stages;
    ConstantBuffer* constant_buffers;
    VertexInputField* vertex_input;
    u32 shader_stages_num;
    u32 vertex_input_num;
    u32 constant_buffers_num;
    RenderBackendPipeline* backend_state;
} PipelineRenderResource;

typedef struct MeshRenderResource
{
    Mesh mesh;
    RenderBackendMesh* backend_state;
} MeshRenderResource;

static const char* render_resource_type_names[] =
{
    "invalid", "shader", "pipeline", "mesh"
};

static RenderResourceType resource_type_from_str(const char* str)
{
    i32 idx = str_eql_arr(str, render_resource_type_names, arrnum(render_resource_type_names));
    check(idx > 0 && idx < RENDER_RESOURCE_TYPE_NUM, "Invalid render resource type");
    return idx;
}

typedef enum RenderResourceFlag
{
    RENDER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT = 0x1
} RenderResourceFlag;

typedef struct RenderResource
{
    hash64 name_hash;
    RenderResourceHandle handle;
    RenderResourceFlag flag;
    void* data;
} RenderResource;

typedef struct RendererState
{
    RendererBackend* rbs;

    // Note that these are _render resources_, they have nothing to do with non-render stuff!
    RenderResource* arr_resources;
    HandleHashMap* resource_name_to_handle;
    HandlePool* resource_handle_pool;
} RendererState;

RendererState* renderer_create(WindowType window_type, void* window_data)
{
    RendererState* rs = mema_zero(sizeof(RendererState));
    rs->resource_handle_pool = handle_pool_create(1, "RenderResourceHandle");
    rs->resource_name_to_handle = handle_hash_map_create();

    for (RenderResourceType s = 1; s < RENDER_RESOURCE_TYPE_NUM; ++s)
        handle_pool_set_type(rs->resource_handle_pool, s, render_resource_type_names[s]);
    
    RendererBackend* rbs = renderer_backend_create(window_type, window_data);
    rs->rbs = rbs;
    return rs;
}

#define get_resource(r, t, h) ((t*)(r + handle_index(h))->data)

static void deinit_resource(RendererState* rs, RenderResourceHandle h)
{
    switch(handle_type(h))
    {
        case RENDER_RESOURCE_TYPE_SHADER: {
            renderer_backend_destroy_shader(rs->rbs, get_resource(rs->arr_resources, ShaderRenderResource, h)->backend_state);
        } break;
        
        case RENDER_RESOURCE_TYPE_PIPELINE: {
            renderer_backend_destroy_pipeline(rs->rbs, get_resource(rs->arr_resources, PipelineRenderResource, h)->backend_state);
        } break;

        case RENDER_RESOURCE_TYPE_MESH: {
            renderer_backend_destroy_mesh(rs->rbs, get_resource(rs->arr_resources, MeshRenderResource, h)->backend_state);
        } break;

        case RENDER_RESOURCE_TYPE_NUM:
        case RENDER_RESOURCE_TYPE_INVALID:
            error("Invalid resource in render resource list"); break;
    }
}

static void init_resource(RendererState* rs, RenderResourceHandle h)
{
    switch(handle_type(h))
    {
        case RENDER_RESOURCE_TYPE_SHADER: {
            ShaderRenderResource* sr = get_resource(rs->arr_resources, ShaderRenderResource, h);
            sr->backend_state = renderer_backend_create_shader(rs->rbs, sr->source, sr->source_size);
        } break;
        
        case RENDER_RESOURCE_TYPE_PIPELINE: {
            PipelineRenderResource* pr = get_resource(rs->arr_resources, PipelineRenderResource, h);

            RenderBackendShader** backend_shader_stages = mema(sizeof(RenderBackendShader*) * pr->shader_stages_num);
            ShaderType* backend_shader_types = mema(sizeof(ShaderType) * pr->shader_stages_num);
            for (u32 shdr_idx = 0; shdr_idx < pr->shader_stages_num; ++shdr_idx)
            {
                const ShaderRenderResource* srr = get_resource(rs->arr_resources, ShaderRenderResource, pr->shader_stages[shdr_idx]);
                backend_shader_stages[shdr_idx] = srr->backend_state;
                backend_shader_types[shdr_idx] = srr->type;
            }

            ShaderDataType* vertex_input_types = mema(sizeof(ShaderDataType) * pr->vertex_input_num);
            for (u32 vi_idx = 0; vi_idx < pr->vertex_input_num; ++vi_idx)
                vertex_input_types[vi_idx] = pr->vertex_input[vi_idx].type;

            u32* constant_buffer_sizes = mema(sizeof(u32) * pr->constant_buffers_num);
            u32* constant_buffer_binding_indices = mema(sizeof(u32) * pr->constant_buffers_num);

            for (u32 cb_idx = 0; cb_idx < pr->constant_buffers_num; ++cb_idx)
            {
                u32 s = 0;

                for (u32 cb_field_idx = 0; cb_field_idx < pr->constant_buffers[cb_idx].fields_num; ++cb_field_idx)
                    s += shader_data_type_size(pr->constant_buffers[cb_idx].fields[cb_field_idx].type);

                constant_buffer_sizes[cb_idx] = s;
                constant_buffer_binding_indices[cb_idx] = pr->constant_buffers[cb_idx].binding;
            }

            pr->backend_state = renderer_backend_create_pipeline(
                rs->rbs, backend_shader_stages, backend_shader_types, pr->shader_stages_num,
                vertex_input_types, pr->vertex_input_num,
                constant_buffer_sizes, constant_buffer_binding_indices, pr->constant_buffers_num);

            memf(constant_buffer_binding_indices);
            memf(constant_buffer_sizes);
            memf(vertex_input_types);
            memf(backend_shader_types);
            memf(backend_shader_stages);

        } break;

        case RENDER_RESOURCE_TYPE_MESH: {
            MeshRenderResource* g = get_resource(rs->arr_resources, MeshRenderResource, h);
            g->backend_state = renderer_backend_create_mesh(rs->rbs, &g->mesh);
        } break;

        case RENDER_RESOURCE_TYPE_NUM:
        case RENDER_RESOURCE_TYPE_INVALID:
            error("Invalid resource in render resource list"); break;
    }
}

static void destroy_resource(RendererState* rs, RenderResourceHandle h)
{
    deinit_resource(rs, h);

    switch(handle_type(h))
    {
        case RENDER_RESOURCE_TYPE_SHADER: {
            memf(get_resource(rs->arr_resources, ShaderRenderResource, h)->source);
        } break;
        
        case RENDER_RESOURCE_TYPE_PIPELINE: {
            const PipelineRenderResource* pr = get_resource(rs->arr_resources, PipelineRenderResource, h);
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

        case RENDER_RESOURCE_TYPE_MESH: {
            MeshRenderResource* g = get_resource(rs->arr_resources, MeshRenderResource, h);
            memf(g->mesh.vertices);
            memf(g->mesh.indices);
        } break;

        case RENDER_RESOURCE_TYPE_NUM:
        case RENDER_RESOURCE_TYPE_INVALID:
            error("Invalid resource in render resource list"); break;
    }

    memf(rs->arr_resources[handle_index(h)].data);
    handle_pool_return(rs->resource_handle_pool, h);
    handle_hash_map_remove(rs->resource_name_to_handle, rs->arr_resources[handle_index(h)].name_hash);
}

static void reinit_resource(RendererState* rs, RenderResourceHandle h)
{
    deinit_resource(rs, h);
    init_resource(rs, h);
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

RenderResourceHandle renderer_resource_load(RendererState* rs, const char* filename)
{
    hash64 name_hash = str_hash(filename);
    ResourceHandle existing = handle_hash_map_get(rs->resource_name_to_handle, name_hash);

    if (existing != HANDLE_INVALID)
        return existing;

    const char* ext = path_ext(filename);
    RenderResourceType type = resource_type_from_str(ext);
    RenderResource r = { .name_hash = name_hash };
    
    info("Loading resource %s", filename);

    switch(type)
    {
        case RENDER_RESOURCE_TYPE_SHADER: {
            #define format_check(cond, msg, ...) (check(cond, "When parsing shader %s: %s", filename, msg, ##__VA_ARGS__))
            ShaderRenderResource sr = {};
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
            r.data = mema_copyt(&sr);
        } break;

        case RENDER_RESOURCE_TYPE_PIPELINE: {
            PipelineRenderResource pr = {};
            #define ensure(expr) if (!(expr)) error("Error in pipeline resource load");
            FileLoadResult flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
            ensure(flr.ok);
            JzonParseResult jpr = jzon_parse(flr.data);
            ensure(jpr.ok && jpr.output.is_table);
            memf(flr.data);

            const JzonValue* jz_shader_stages = jzon_get(&jpr.output, "shader_stages");
            ensure(jz_shader_stages && jz_shader_stages->is_array);
            pr.shader_stages_num = jz_shader_stages->size;
            pr.shader_stages = mema_zero(sizeof(ShaderRenderResource) * pr.shader_stages_num);
            
            for (u32 shdr_idx = 0; shdr_idx < jz_shader_stages->size; ++shdr_idx)
            {
                const JzonValue* jz_shader_stage = jz_shader_stages->array_val + shdr_idx;
                ensure(jz_shader_stage->is_string);
                pr.shader_stages[shdr_idx] = renderer_resource_load(rs, jz_shader_stage->string_val);
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

            r.flag = RENDER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT;
            r.data = mema_copyt(&pr);
        } break;


        case RENDER_RESOURCE_TYPE_MESH: {
            #define ensure(expr) if (!(expr)) error("Error in pipeline resource load");
            FileLoadResult flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
            ensure(flr.ok);
            JzonParseResult jpr = jzon_parse(flr.data);
            ensure(jpr.ok && jpr.output.is_table);
            memf(flr.data);

            const JzonValue* jz_source = jzon_get(&jpr.output, "source");
            ensure(jz_source && jz_source->is_string);

            ObjLoadResult olr = obj_load(jz_source->string_val);
            check(olr.ok, "Failed loading obj");
            jzon_free(&jpr.output);

            MeshRenderResource g = {
                .mesh = olr.mesh
            };

            mema_repln(g.mesh.vertices, g.mesh.vertices_num);
            mema_repln(g.mesh.indices, g.mesh.indices_num);
            memf(olr.mesh.vertices);
            memf(olr.mesh.indices);

            r.data = mema_copyt(&g);
        } break;

        default: error("Implement me!"); break;
    }

    RenderResourceHandle h = handle_pool_borrow(rs->resource_handle_pool, type);
    r.handle = h;
    array_fill_and_set(rs->arr_resources, handle_index(h), r);
    init_resource(rs, h);
    return h;
}

void renderer_destroy(RendererState* rs)
{
    info("Destroying render");
    renderer_backend_wait_until_idle(rs->rbs);
    
    info("Destroying all render resources");
    for (size_t i = 0; i < array_num(rs->arr_resources); ++i)
        destroy_resource(rs, rs->arr_resources[i].handle);

    array_destroy(rs->arr_resources);
    renderer_backend_destroy(rs->rbs);
    handle_hash_map_destroy(rs->resource_name_to_handle);
    handle_pool_destroy(rs->resource_handle_pool);
    memf(rs);
}

static void populate_constant_buffers(RendererState* rs, const PipelineRenderResource* pr, const Mat4* model_matrix, const Mat4* mvp_matrix)
{
    for (u32 cb_idx = 0; cb_idx < pr->constant_buffers_num; ++cb_idx)
    {
        const ConstantBuffer* cb = pr->constant_buffers + cb_idx;

        u32 offset = 0;

        for (u32 cbf_idx = 0; cbf_idx < cb->fields_num; ++cbf_idx)
        {
            const ConstantBufferField* cbf = cb->fields + cbf_idx;

            switch(cbf->auto_value)
            {
                case CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL:
                    renderer_backend_update_constant_buffer(rs->rbs, pr->backend_state, cb->binding, model_matrix, sizeof(*model_matrix), offset);
                    break;

                case CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL_VIEW_PROJECTION:
                    renderer_backend_update_constant_buffer(rs->rbs, pr->backend_state, cb->binding, mvp_matrix, sizeof(*mvp_matrix), offset);
                    break;

                default: break;
            }

            offset += shader_data_type_size(cbf->type);
        }

    }
}

void renderer_draw(RendererState* rs, RenderResourceHandle pipeline_handle, RenderResourceHandle mesh_handle, const Vec3* cam_pos, const Quat* cam_rot)
{
    Mat4 camera_matrix = mat4_from_rotation_and_translation(cam_rot, cam_pos);
    Mat4 view_matrix = mat4_inverse(&camera_matrix);

    Mat4 model_matrix = mat4_identity();

    Vec2u size = renderer_backend_get_size(rs->rbs);
    Mat4 proj_matrix = mat4_create_projection_matrix(size.x, size.y);
    Mat4 proj_view_matrix = mat4_mul(&view_matrix, &proj_matrix);
    Mat4 mvp_matrix = mat4_mul(&model_matrix, &proj_view_matrix);

    const PipelineRenderResource* pipeline = get_resource(rs->arr_resources, PipelineRenderResource, pipeline_handle);
    populate_constant_buffers(rs, pipeline, &model_matrix, &mvp_matrix);
    renderer_backend_draw(rs->rbs, pipeline->backend_state, get_resource(rs->arr_resources, MeshRenderResource, mesh_handle)->backend_state);
}

void renderer_present(RendererState* rs)
{
    renderer_backend_present(rs->rbs);
}

void renderer_wait_for_new_frame(RendererState* rs)
{
    renderer_backend_wait_for_new_frame(rs->rbs);
}

void renderer_surface_resized(RendererState* rs, u32 w, u32 h)
{
    info("Render resizing to %d x %d", w, h);
    renderer_backend_wait_until_idle(rs->rbs);
    renderer_backend_surface_resized(rs->rbs, w, h);

    for (size_t i = 0; i < array_num(rs->arr_resources); ++i)
    {
        RenderResource* rr = rs->arr_resources + i;

        if (rr->flag & RENDER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT)
            reinit_resource(rs, rr->handle);
    }
    renderer_backend_wait_until_idle(rs->rbs);
}