#include "renderer.h"
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

struct ShaderRenderResource
{
    char* source;
    u64 source_size;
    ShaderType type;
    RenderBackendShader* backend_state;
};

struct PipelineRenderResource
{
    RenderResourceHandle* shader_stages;
    ConstantBuffer* constant_buffers;
    VertexInputField* vertex_input;
    u32 shader_stages_num;
    u32 vertex_input_num;
    u32 constant_buffers_num;
    RenderBackendPipeline* backend_state;
};

struct MeshRenderResource
{
    Mesh mesh;
    RenderBackendMesh* backend_state;
};

struct WorldObject {
    Mat4 model;
    RenderResourceHandle mesh;
};

struct WorldRenderResource {
    Array<WorldObject> objects;
    Array<size_t> free_object_indices; // holes in objects
};

static char* render_resource_type_names[] =
{
    "invalid", "shader", "pipeline", "mesh", "world"
};

static RenderResourceType resource_type_from_str(char* str)
{
    i32 idx = str_eql_arr(str, render_resource_type_names, arrnum(render_resource_type_names));
    check(idx > 0 && idx < (i32)RenderResourceType::Num, "Invalid render resource type");
    return (RenderResourceType)idx;
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

struct Renderer
{
    RendererBackend* rbs;

    // Note that these are _render resources_, they have nothing to do with non-render stuff!
    Array<RenderResource> resources;
    HandleHashMap* resource_name_to_handle;
    HandlePool* resource_handle_pool;
};

Renderer* renderer_create(WindowType window_type, void* window_data)
{
    Renderer* rs = mema_zero_t(Renderer);
    rs->resource_handle_pool = handle_pool_create(1, "RenderResourceHandle");
    rs->resource_name_to_handle = handle_hash_map_create();

    for (u32 s = 1; s < (u32)RenderResourceType::Num; ++s)
        handle_pool_set_type(rs->resource_handle_pool, s, render_resource_type_names[s]);
    
    RendererBackend* rbs = renderer_backend_create(window_type, window_data);
    rs->rbs = rbs;
    return rs;
}

#define get_resource(r, t, h) ((t*)(r[handle_index(h)]).data)

static void deinit_resource(mut Renderer* rs, RenderResourceHandle h)
{
    switch((RenderResourceType)handle_type(h))
    {
        case RenderResourceType::Shader: {
            renderer_backend_destroy_shader(rs->rbs, get_resource(rs->resources, ShaderRenderResource, h)->backend_state);
        } break;
        
        case RenderResourceType::Pipeline: {
            renderer_backend_destroy_pipeline(rs->rbs, get_resource(rs->resources, PipelineRenderResource, h)->backend_state);
        } break;

        case RenderResourceType::Mesh: {
            renderer_backend_destroy_mesh(rs->rbs, get_resource(rs->resources, MeshRenderResource, h)->backend_state);
        } break;

        case RenderResourceType::World: {} break;

        default: error("Implement me!");
    }
}

static void init_resource(mut Renderer* rs, RenderResourceHandle h)
{
    switch((RenderResourceType)handle_type(h))
    {
        case RenderResourceType::Shader: {
            ShaderRenderResource* sr = get_resource(rs->resources, ShaderRenderResource, h);
            sr->backend_state = renderer_backend_create_shader(rs->rbs, sr->source, sr->source_size);
        } break;
        
        case RenderResourceType::Pipeline: {
            PipelineRenderResource* pr = get_resource(rs->resources, PipelineRenderResource, h);

            RenderBackendShader** backend_shader_stages = mema_tn(RenderBackendShader*, pr->shader_stages_num);
            ShaderType* backend_shader_types = mema_tn(ShaderType, pr->shader_stages_num);
            for (u32 shdr_idx = 0; shdr_idx < pr->shader_stages_num; ++shdr_idx)
            {
                ShaderRenderResource* srr = get_resource(rs->resources, ShaderRenderResource, pr->shader_stages[shdr_idx]);
                backend_shader_stages[shdr_idx] = srr->backend_state;
                backend_shader_types[shdr_idx] = srr->type;
            }

            ShaderDataType* vertex_input_types = mema_tn(ShaderDataType, pr->vertex_input_num);
            for (u32 vi_idx = 0; vi_idx < pr->vertex_input_num; ++vi_idx)
                vertex_input_types[vi_idx] = pr->vertex_input[vi_idx].type;

            u32* constant_buffer_sizes = mema_tn(u32, pr->constant_buffers_num);
            u32* constant_buffer_binding_indices = mema_tn(u32, pr->constant_buffers_num);

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

        case RenderResourceType::Mesh: {
            MeshRenderResource* g = get_resource(rs->resources, MeshRenderResource, h);
            g->backend_state = renderer_backend_create_mesh(rs->rbs, &g->mesh);
        } break;

        default: error("Implement me!");
    }
}

static void destroy_resource(mut Renderer* rs, RenderResourceHandle h)
{
    deinit_resource(rs, h);

    switch((RenderResourceType)handle_type(h))
    {
        case RenderResourceType::Shader: {
            memf(get_resource(rs->resources, ShaderRenderResource, h)->source);
        } break;
        
        case RenderResourceType::Pipeline: {
            PipelineRenderResource* pr = get_resource(rs->resources, PipelineRenderResource, h);
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

        case RenderResourceType::Mesh: {
            MeshRenderResource* g = get_resource(rs->resources, MeshRenderResource, h);
            memf(g->mesh.vertices);
            memf(g->mesh.indices);
        } break;

        case RenderResourceType::World: {
            WorldRenderResource* w = get_resource(rs->resources, WorldRenderResource, h);
            array_destroy(&w->objects);
        } break;

        default: error("Invalid resource in render resource list");
    }

    memf(rs->resources[handle_index(h)].data);
    handle_pool_return(rs->resource_handle_pool, h);
    handle_hash_map_remove(rs->resource_name_to_handle, rs->resources[handle_index(h)].name_hash);
}

static void reinit_resource(mut Renderer* rs, RenderResourceHandle h)
{
    deinit_resource(rs, h);
    init_resource(rs, h);
}

static ShaderDataType shader_data_type_str_to_enum(char* str)
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

static ConstantBufferAutoValue cb_autoval_str_to_enum(char* str)
{
    if (str_eql(str, "mat_model_view_projection"))
        return ConstantBufferAutoValue::MatModelViewProjection;

    if (str_eql(str, "mat_model"))
        return ConstantBufferAutoValue::MatModel;

    if (str_eql(str, "mat_projection"))
        return ConstantBufferAutoValue::MatProjection;

    if (str_eql(str, "mat_view_projection"))
        return ConstantBufferAutoValue::MatViewProjection;

    return ConstantBufferAutoValue::None;
}

static VertexInputValue il_val_str_to_enum(char* str)
{
    if (str_eql(str, "position"))
        return VertexInputValue::Position;

    if (str_eql(str, "normal"))
        return VertexInputValue::Normal;

    if (str_eql(str, "texcoord"))
        return VertexInputValue::Texcoord;

    if (str_eql(str, "color"))
        return VertexInputValue::Color;

    return VertexInputValue::Invalid;
}

static ShaderType shader_type_str_to_enum(char* str)
{
    if (str_eql(str, "vertex"))
        return ShaderType::Vertex;

    if (str_eql(str, "fragment"))
        return ShaderType::Fragment;

    return ShaderType::Invalid;
}

RenderResourceHandle renderer_resource_load(mut Renderer* rs, char* filename)
{
    hash64 name_hash = str_hash(filename);
    RenderResourceHandle existing = handle_hash_map_get(rs->resource_name_to_handle, name_hash);

    if (existing != HANDLE_INVALID)
        return existing;

    char* ext = path_ext(filename);
    RenderResourceType type = resource_type_from_str(ext);
    RenderResource r = { .name_hash = name_hash };
    
    info("Loading resource %s", filename);

    switch(type)
    {
        case RenderResourceType::Shader: {
            #define format_check(cond, msg, ...) (check(cond, "When parsing shader %s: %s", filename, msg, ##__VA_ARGS__))
            ShaderRenderResource sr = {};
            FileLoadResult shader_flr = file_load(filename, FileLoadMode::NullTerminated);
            format_check(shader_flr.ok, "File missing");
            JzonParseResult jpr = jzon_parse((char*)shader_flr.data);
            format_check(jpr.ok && jpr.output.is_table, "Malformed shader");
            memf(shader_flr.data);
            
            JzonValue* jz_type = jzon_get(&jpr.output, "type");
            format_check(jz_type && jz_type->is_string, "type not a string or missing");
            ShaderType st = shader_type_str_to_enum(jz_type->string_val);
            format_check(st != ShaderType::Invalid, "type isn't an allowed value");
            sr.type = st;

            JzonValue* jz_source = jzon_get(&jpr.output, "source");
            format_check(jz_source && jz_source->is_string, "source missing or not a string");

            FileLoadResult source_flr = file_load(jz_source->string_val, FileLoadMode::Default);
            format_check(source_flr.ok, "failed opening shader source %s", jz_source->string_val);
            sr.source = (char*)mema_copy(source_flr.data, source_flr.data_size);
            sr.source_size = source_flr.data_size;
            memf(source_flr.data);
            jzon_free(&jpr.output);
            r.data = mema_copy_t(&sr, ShaderRenderResource);
        } break;

        case RenderResourceType::Pipeline: {
            PipelineRenderResource pr = {};
            #define ensure(expr) if (!(expr)) error("Error in pipeline resource load");
            FileLoadResult flr = file_load(filename, FileLoadMode::NullTerminated);
            ensure(flr.ok);
            JzonParseResult jpr = jzon_parse((char*)flr.data);
            ensure(jpr.ok && jpr.output.is_table);
            memf(flr.data);

            JzonValue* jz_shader_stages = jzon_get(&jpr.output, "shader_stages");
            ensure(jz_shader_stages && jz_shader_stages->is_array);
            pr.shader_stages_num = jz_shader_stages->size;
            pr.shader_stages = mema_zero_tn(RenderResourceHandle, pr.shader_stages_num);
            
            for (u32 shdr_idx = 0; shdr_idx < jz_shader_stages->size; ++shdr_idx)
            {
                JzonValue* jz_shader_stage = jz_shader_stages->array_val + shdr_idx;
                ensure(jz_shader_stage->is_string);
                pr.shader_stages[shdr_idx] = renderer_resource_load(rs, jz_shader_stage->string_val);
            }

            JzonValue* jz_constant_buffers = jzon_get(&jpr.output, "constant_buffers");

            if (jz_constant_buffers)
            {
                ensure(jz_constant_buffers->is_array);
                pr.constant_buffers_num = jz_constant_buffers->num;
                pr.constant_buffers = mema_zero_tn(ConstantBuffer, pr.constant_buffers_num);

                for (u32 cb_idx = 0; cb_idx < pr.constant_buffers_num; ++cb_idx)
                {
                    JzonValue* jz_constant_buffer = jz_constant_buffers->array_val + cb_idx;
                    ensure(jz_constant_buffer->is_table);

                    JzonValue* jz_binding = jzon_get(jz_constant_buffer, "binding");
                    ensure(jz_binding && jz_binding->is_int);
                    pr.constant_buffers[cb_idx].binding = jz_binding->int_val;

                    JzonValue* jz_fields = jzon_get(jz_constant_buffer, "fields");
                    ensure(jz_fields && jz_fields->is_array);

                    pr.constant_buffers[cb_idx].fields_num = (u32)jz_fields->num;
                    pr.constant_buffers[cb_idx].fields = mema_zero_tn(ConstantBufferField, pr.constant_buffers[cb_idx].fields_num);
                    for (u32 i = 0; i < jz_fields->size; ++i)
                    {
                        ConstantBufferField* cbf = pr.constant_buffers[cb_idx].fields + i;
                        JzonValue* jz_cbf = jz_fields->array_val + i;
                        ensure(jz_cbf->is_table);

                        JzonValue* jz_name = jzon_get(jz_cbf, "name");
                        ensure(jz_name && jz_name->is_string);
                        cbf->name = str_copy(jz_name->string_val);

                        JzonValue* jz_type = jzon_get(jz_cbf, "type");
                        ensure(jz_type && jz_type->is_string)
                        ShaderDataType sdt = shader_data_type_str_to_enum(jz_type->string_val);
                        ensure(sdt != ShaderDataType::Invalid);
                        cbf->type = sdt;

                        JzonValue* jz_cbf_autoval = jzon_get(jz_cbf, "value");
                        
                        if (jz_cbf_autoval && jz_cbf_autoval->is_string)
                        {
                            ConstantBufferAutoValue auto_val = cb_autoval_str_to_enum(jz_cbf_autoval->string_val);
                            cbf->auto_value = auto_val;
                        }
                    }
                }
            }    

            JzonValue* jz_vertex_input = jzon_get(&jpr.output, "vertex_input");

            if (jz_vertex_input)
            {
                ensure(jz_vertex_input && jz_vertex_input->is_array);
                pr.vertex_input_num = (u32)jz_vertex_input->size;
                pr.vertex_input = mema_zero_tn(VertexInputField, pr.vertex_input_num);
                for (u32 i = 0; i < jz_vertex_input->size; ++i)
                {
                    VertexInputField* vif = &pr.vertex_input[i];
                    JzonValue* jz_vif = jz_vertex_input->array_val + i;
                    ensure(jz_vif && jz_vif->is_table);

                    JzonValue* jz_name = jzon_get(jz_vif, "name");
                    ensure(jz_name && jz_name->is_string);
                    vif->name = str_copy(jz_name->string_val);

                    JzonValue* jz_type = jzon_get(jz_vif, "type");
                    ensure(jz_type && jz_type->is_string);
                    ShaderDataType sdt = shader_data_type_str_to_enum(jz_type->string_val);
                    ensure(sdt != ShaderDataType::Invalid);
                    vif->type = sdt;

                    JzonValue* jz_vif_val = jzon_get(jz_vif, "value");
                    ensure(jz_vif_val && jz_vif_val->is_string);
                    VertexInputValue val = il_val_str_to_enum(jz_vif_val->string_val);
                    ensure(val != VertexInputValue::Invalid);
                    vif->value = val;
                }
            }

            jzon_free(&jpr.output);

            r.flag = RENDER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT;
            r.data = mema_copy_t(&pr, PipelineRenderResource);
        } break;


        case RenderResourceType::Mesh: {
            #define ensure(expr) if (!(expr)) error("Error in pipeline resource load");
            FileLoadResult flr = file_load(filename, FileLoadMode::NullTerminated);
            ensure(flr.ok);
            JzonParseResult jpr = jzon_parse((char*)flr.data);
            ensure(jpr.ok && jpr.output.is_table);
            memf(flr.data);

            JzonValue* jz_source = jzon_get(&jpr.output, "source");
            ensure(jz_source && jz_source->is_string);

            ObjLoadResult olr = obj_load(jz_source->string_val);
            check(olr.ok, "Failed loading obj");
            jzon_free(&jpr.output);

            MeshRenderResource m = {
                .mesh = olr.mesh
            };

            mema_repln(m.mesh.vertices, m.mesh.vertices_num);
            mema_repln(m.mesh.indices, m.mesh.indices_num);
            memf(olr.mesh.vertices);
            memf(olr.mesh.indices);

            r.data = mema_copy_t(&m, MeshRenderResource);
        } break;

        default: error("Implement me!"); break;
    }

    RenderResourceHandle h = handle_pool_borrow(rs->resource_handle_pool, (u32)type);
    r.handle = h;
    array_fill_and_set(&rs->resources, r, handle_index(h));
    init_resource(rs, h);
    return h;
}

void renderer_destroy(mut Renderer* rs)
{
    info("Destroying render");
    renderer_backend_wait_until_idle(rs->rbs);
    
    info("Destroying all render resources");
    for (size_t i = 0; i < rs->resources.num; ++i)
        destroy_resource(rs, rs->resources[i].handle);

    array_destroy(&rs->resources);
    renderer_backend_destroy(rs->rbs);
    handle_hash_map_destroy(rs->resource_name_to_handle);
    handle_pool_destroy(rs->resource_handle_pool);
    memf(rs);
}

RenderResourceHandle renderer_create_world(mut Renderer* rs)
{
    RenderResourceHandle h = handle_pool_borrow(rs->resource_handle_pool, (u32)RenderResourceType::World);
    RenderResource r = {
        .handle = h,
        .data = mema_t(WorldRenderResource)
    };
    array_fill_and_set(&rs->resources, r, handle_index(h));
    return h;
}

void renderer_destroy_world(mut Renderer* rs, RenderResourceHandle h)
{
    destroy_resource(rs, h);
}

size_t renderer_world_add(mut Renderer* rs, RenderResourceHandle world, RenderResourceHandle mesh, Mat4* model)
{
    WorldRenderResource* w = get_resource(rs->resources, WorldRenderResource, world);

    if (w->free_object_indices.num > 0)
    {
        size_t idx = array_pop(&w->free_object_indices);
        w->objects[idx].mesh = mesh;
        w->objects[idx].model = *model;
        return idx;
    }

    size_t idx = w->objects.num;

    WorldObject wo = {
        .mesh = mesh,
        .model = *model
    };

    array_push(&w->objects, wo);
    return idx;
}

void renderer_world_remove(mut Renderer* rs, RenderResourceHandle world, size_t idx)
{
    WorldRenderResource* w = get_resource(rs->resources, WorldRenderResource, world);
    check(w->objects[idx].mesh == HANDLE_INVALID, "Trying to remove from world twice");
    w->objects[idx].mesh = HANDLE_INVALID;
    array_push(&w->free_object_indices, idx);
}

void renderer_world_move(mut Renderer* rs, RenderResourceHandle world, u32 idx, Mat4* model)
{
    WorldRenderResource* w = get_resource(rs->resources, WorldRenderResource, world);
    w->objects[idx].model = *model;
}

static void populate_constant_buffers(Renderer* rs, PipelineRenderResource* pr, Mat4* model_matrix, Mat4* mvp_matrix)
{
    for (u32 cb_idx = 0; cb_idx < pr->constant_buffers_num; ++cb_idx)
    {
        ConstantBuffer* cb = pr->constant_buffers + cb_idx;

        u32 offset = 0;

        for (u32 cbf_idx = 0; cbf_idx < cb->fields_num; ++cbf_idx)
        {
            ConstantBufferField* cbf = cb->fields + cbf_idx;

            switch(cbf->auto_value)
            {
                case ConstantBufferAutoValue::MatModel:
                    renderer_backend_update_constant_buffer(rs->rbs, pr->backend_state, cb->binding, model_matrix, sizeof(*model_matrix), offset);
                    break;

                case ConstantBufferAutoValue::MatModelViewProjection:
                    renderer_backend_update_constant_buffer(rs->rbs, pr->backend_state, cb->binding, mvp_matrix, sizeof(*mvp_matrix), offset);
                    break;

                default: break;
            }

            offset += shader_data_type_size(cbf->type);
        }

    }
}

void renderer_draw(Renderer* rs, RenderResourceHandle pipeline_handle, RenderResourceHandle mesh_handle, Mat4* model, Vec3* cam_pos, Quat* cam_rot)
{
    Mat4 camera_matrix = mat4_from_rotation_and_translation(cam_rot, cam_pos);
    Mat4 view_matrix = mat4_inverse(&camera_matrix);

    Vec2u size = renderer_backend_get_size(rs->rbs);
    Mat4 proj_matrix = mat4_create_projection_matrix(size.x, size.y);
    Mat4 mvp_matrix = (*model) * view_matrix * proj_matrix;

    PipelineRenderResource* pipeline = get_resource(rs->resources, PipelineRenderResource, pipeline_handle);
    populate_constant_buffers(rs, pipeline, model, &mvp_matrix);
    renderer_backend_draw(rs->rbs, pipeline->backend_state, get_resource(rs->resources, MeshRenderResource, mesh_handle)->backend_state);
}

void renderer_draw_world(Renderer* rs, RenderResourceHandle pipeline_handle, RenderResourceHandle world_handle, Vec3* cam_pos, Quat* cam_rot)
{
    WorldRenderResource* w = get_resource(rs->resources, WorldRenderResource, world_handle);

    for (size_t i = 0; i < w->objects.num; ++i)
    {
        WorldObject* obj = w->objects.data + i;

        if (obj->mesh == HANDLE_INVALID)
            continue;

        renderer_draw(rs, pipeline_handle, obj->mesh, &obj->model, cam_pos, cam_rot);
    }
}

void renderer_present(mut Renderer* rs)
{
    renderer_backend_present(rs->rbs);
}

void renderer_wait_for_new_frame(Renderer* rs)
{
    renderer_backend_wait_for_new_frame(rs->rbs);
}

void renderer_surface_resized(mut Renderer* rs, u32 w, u32 h)
{
    info("Render resizing to %d x %d", w, h);
    renderer_backend_wait_until_idle(rs->rbs);
    renderer_backend_surface_resized(rs->rbs, w, h);

    for (size_t i = 0; i < rs->resources.num; ++i)
    {
        RenderResource* rr = rs->resources.data + i;

        if (rr->flag & RENDER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT)
            reinit_resource(rs, rr->handle);
    }
    renderer_backend_wait_until_idle(rs->rbs);
}