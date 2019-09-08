#include "renderer.h"
#include "renderer_backend.h"
#include "memory.h"
#include "handle_pool.h"
#include "dynamic_array.h"
#include "log.h"
#include "render_resource.h"
#include "str.h"
#include "handle_hash_map.h"
#include "path.h"
#include "file.h"
#include "jzon.h"
#include "obj_loader.h"
#include "handle.h"

struct ShaderRenderResource
{
    char* source;
    u64 source_size;
    ShaderType type;
    RenderBackendShader* backend_state;
    ConstantBufferField* push_constant_fields;
    u32 push_constant_fields_num;
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

struct WorldObject
{
    Mat4 model;
    RenderResourceHandle mesh;
};

struct WorldRenderResource
{
    WorldObject* objects; // dynamic
    u32* free_object_indices; // dynamic, holes in objects
};

static const char* render_resource_type_names[] =
{
    "invalid", "shader", "pipeline", "mesh", "world"
};

static RenderResourceType resource_type_from_str(const char* str)
{
    i32 idx = str_eql_arr(str, render_resource_type_names, sizeof(render_resource_type_names)/sizeof(render_resource_type_names[0]));
    check(idx > 0 && idx < RENDER_RESOURCE_TYPE_NUM, "Invalid render resource type");
    return (RenderResourceType)idx;
}

enum RenderResourceFlag
{
    RENDER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT = 0x1
};

struct RenderResource
{
    hash64 name_hash;
    RenderResourceHandle handle; // type is in the handle, get with handle_type(handle)
    RenderResourceFlag flag;
    void* data;
};

struct Renderer
{
    RenderResource* resources;
    u32 resources_num;
    HandleHashMap* resource_name_to_handle;
    HandlePool* resource_handle_pool;
    RenderResourceHandle debug_draw_pipeline;
};

static Renderer rs = {};
static bool inited = false;

void renderer_init(WindowType window_type, const GenericWindowInfo& window_info)
{
    check(!inited, "Trying to init renderer twice!");
    inited = true;
    rs.resource_handle_pool = handle_pool_create(HANDLE_POOL_TYPE_RENDER_RESOURCE);
    rs.resource_name_to_handle = handle_hash_map_create();

    for (u32 s = 1; s < RENDER_RESOURCE_TYPE_NUM; ++s)
        handle_pool_set_type(rs.resource_handle_pool, s, render_resource_type_names[s]);
    
    renderer_backend_init(window_type, window_info);
    rs.debug_draw_pipeline = renderer_load_resource("pipeline_debug_draw.pipeline");
}

static void* get_resource_data(RenderResourceHandle h)
{
    check(handle_pool_is_valid(rs.resource_handle_pool, h), "Trying to get RenderResource data, but the passed Handle is invalid");
    return rs.resources[handle_index(h)].data;
}

#define get_resource(t, h) ((t*)get_resource_data(h))

static void deinit_resource(RenderResourceHandle h)
{
    switch(handle_type(h))
    {
        case RENDER_RESOURCE_TYPE_SHADER: {
            renderer_backend_destroy_shader(get_resource(ShaderRenderResource, h)->backend_state);
        } break;
        
        case RENDER_RESOURCE_TYPE_PIPELINE: {
            renderer_backend_destroy_pipeline(get_resource(PipelineRenderResource, h)->backend_state);
        } break;

        case RENDER_RESOURCE_TYPE_MESH: {
            renderer_backend_destroy_mesh(get_resource(MeshRenderResource, h)->backend_state);
        } break;

        case RENDER_RESOURCE_TYPE_WORLD: {} break;

        default: error("Implement me!");
    }
}

static void init_resource(RenderResourceHandle h)
{
    switch(handle_type(h))
    {
        case RENDER_RESOURCE_TYPE_SHADER: {
            ShaderRenderResource* sr = get_resource(ShaderRenderResource, h);
            sr->backend_state = renderer_backend_create_shader(sr->source, sr->source_size);
        } break;
        
        case RENDER_RESOURCE_TYPE_PIPELINE: {
            PipelineRenderResource* pr = get_resource(PipelineRenderResource, h);

            RenderBackendShader** backend_shader_stages = mema_tn(RenderBackendShader*, pr->shader_stages_num);
            ShaderType* backend_shader_types = mema_tn(ShaderType, pr->shader_stages_num);
            u32 push_constants_num = 0;
            u32* push_constants_sizes = NULL;
            ShaderType* push_constants_shader_types = NULL;

            for (u32 shdr_idx = 0; shdr_idx < pr->shader_stages_num; ++shdr_idx)
            {
                ShaderRenderResource* srr = get_resource(ShaderRenderResource, pr->shader_stages[shdr_idx]);
                backend_shader_stages[shdr_idx] = srr->backend_state;
                backend_shader_types[shdr_idx] = srr->type;

                if (srr->push_constant_fields_num > 0)
                {
                    u32 s = 0;

                    for (u32 pc_field_idx = 0; pc_field_idx < srr->push_constant_fields_num; ++pc_field_idx)
                        s += shader_data_type_size(srr->push_constant_fields[pc_field_idx].type);

                    da_push(push_constants_shader_types, srr->type);
                    da_push(push_constants_sizes, s);
                    ++push_constants_num;
                }
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
                backend_shader_stages, backend_shader_types, pr->shader_stages_num,
                vertex_input_types, pr->vertex_input_num,
                constant_buffer_sizes, constant_buffer_binding_indices, pr->constant_buffers_num,
                push_constants_sizes, push_constants_shader_types, push_constants_num);

            da_free(push_constants_sizes);
            da_free(push_constants_shader_types);
            memf(constant_buffer_binding_indices);
            memf(constant_buffer_sizes);
            memf(vertex_input_types);
            memf(backend_shader_types);
            memf(backend_shader_stages);

        } break;

        case RENDER_RESOURCE_TYPE_MESH: {
            MeshRenderResource* g = get_resource(MeshRenderResource, h);
            g->backend_state = renderer_backend_create_mesh(&g->mesh);
        } break;

        default: error("Implement me!");
    }
}

static void destroy_resource(RenderResourceHandle h)
{
    check(handle_pool_is_valid(rs.resource_handle_pool, h), "When trying to destroy RenderResource; the passed Handle was invalid");
    deinit_resource(h);

    switch(handle_type(h))
    {
        case RENDER_RESOURCE_TYPE_SHADER: {
            let sr = get_resource(ShaderRenderResource, h);

            for (u32 i = 0; i < sr->push_constant_fields_num; ++i)
                    memf(sr->push_constant_fields[i].name);

            memf(sr->push_constant_fields);
            memf(sr->source);
        } break;
        
        case RENDER_RESOURCE_TYPE_PIPELINE: {
            PipelineRenderResource* pr = get_resource(PipelineRenderResource, h);
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
            MeshRenderResource* g = get_resource(MeshRenderResource, h);
            memf(g->mesh.vertices);
            memf(g->mesh.indices);
        } break;

        case RENDER_RESOURCE_TYPE_WORLD: {
            WorldRenderResource* w = get_resource(WorldRenderResource, h);
            da_free(w->objects);
        } break;

        default: error("Invalid resource in render resource list");
    }

    memf(rs.resources[handle_index(h)].data);
    handle_pool_return(rs.resource_handle_pool, h);
    handle_hash_map_remove(rs.resource_name_to_handle, rs.resources[handle_index(h)].name_hash);
}

static void reinit_resource(RenderResourceHandle h)
{
    deinit_resource(h);
    init_resource(h);
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

static ConstantBufferField resource_load_parse_constant_buffer_field(const JzonValue& in)
{
    check(in.is_table, "Trying to load constant buffer field, but input Jzon isn't a table.");

    let jz_name = jzon_get(in, "name");
    check(jz_name && jz_name->is_string, "Constant buffer field missing name or name isn't string.");
    char* name = str_copy(jz_name->string_val);

    let jz_type = jzon_get(in, "type");
    check(jz_type && jz_type->is_string, "Constant buffer field missing type or isn't string.");
    ShaderDataType sdt = shader_data_type_str_to_enum(jz_type->string_val);
    check(sdt != SHADER_DATA_TYPE_INVALID, "Constant buffer field of type %s didn't resolve to any internal shader data type.", jz_type->string_val);

    let jz_autoval = jzon_get(in, "value");
    ConstantBufferAutoValue auto_val = (jz_autoval && jz_autoval->is_string) ? cb_autoval_str_to_enum(jz_autoval->string_val) : CONSTANT_BUFFER_AUTO_VALUE_NONE;

    return {
        .name = name,
        .type = sdt,
        .auto_value = auto_val
    };
}

RenderResourceHandle renderer_load_resource(const char* filename)
{
    hash64 name_hash = str_hash(filename);
    RenderResourceHandle existing = handle_hash_map_get(rs.resource_name_to_handle, name_hash);

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
            JzonParseResult jpr = jzon_parse((char*)shader_flr.data);
            format_check(jpr.ok && jpr.output.is_table, "Malformed shader");
            memf(shader_flr.data);
            
            let jz_type = jzon_get(jpr.output, "type");
            format_check(jz_type && jz_type->is_string, "type not a string or missing");
            ShaderType st = shader_type_str_to_enum(jz_type->string_val);
            format_check(st != SHADER_TYPE_INVALID, "type isn't an allowed value");
            sr.type = st;

            let jz_source = jzon_get(jpr.output, "source");
            format_check(jz_source && jz_source->is_string, "source missing or not a string");

            let jz_push_constant = jzon_get(jpr.output, "push_constant");

            if (jz_push_constant)
            {
                check(jz_push_constant->is_array, "push_constant isn't array");
                sr.push_constant_fields_num  = (u32)jz_push_constant->num;
                sr.push_constant_fields = mema_zero_tn(ConstantBufferField, sr.push_constant_fields_num);

                for (u32 i = 0; i < sr.push_constant_fields_num; ++i)
                    sr.push_constant_fields[i] = resource_load_parse_constant_buffer_field(jz_push_constant->array_val[i]);
            }

            FileLoadResult source_flr = file_load(jz_source->string_val);
            format_check(source_flr.ok, "failed opening shader source %s", jz_source->string_val);
            sr.source = (char*)mema_copy(source_flr.data, source_flr.data_size);
            sr.source_size = source_flr.data_size;
            memf(source_flr.data);
            jzon_free(&jpr.output);
            r.data = mema_copy_t(&sr, ShaderRenderResource);
        } break;

        case RENDER_RESOURCE_TYPE_PIPELINE: {
            PipelineRenderResource pr = {};
            #define ensure(expr) if (!(expr)) error("Error in pipeline resource load");
            FileLoadResult flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
            ensure(flr.ok);
            JzonParseResult jpr = jzon_parse((char*)flr.data);
            ensure(jpr.ok && jpr.output.is_table);
            memf(flr.data);

            let jz_shader_stages = jzon_get(jpr.output, "shader_stages");
            ensure(jz_shader_stages && jz_shader_stages->is_array);
            pr.shader_stages_num = jz_shader_stages->size;
            pr.shader_stages = mema_zero_tn(RenderResourceHandle, pr.shader_stages_num);
            
            for (u32 shdr_idx = 0; shdr_idx < jz_shader_stages->size; ++shdr_idx)
            {
                let jz_shader_stage = jz_shader_stages->array_val + shdr_idx;
                ensure(jz_shader_stage->is_string);
                pr.shader_stages[shdr_idx] = renderer_load_resource(jz_shader_stage->string_val);
            }

            let jz_constant_buffers = jzon_get(jpr.output, "constant_buffers");

            if (jz_constant_buffers)
            {
                ensure(jz_constant_buffers->is_array);

                pr.constant_buffers_num = jz_constant_buffers->num;
                pr.constant_buffers = mema_zero_tn(ConstantBuffer, pr.constant_buffers_num);

                for (u32 cb_idx = 0; cb_idx < pr.constant_buffers_num; ++cb_idx)
                {
                    JzonValue jz_constant_buffer = jz_constant_buffers->array_val[cb_idx];
                    ensure(jz_constant_buffer.is_table);

                    let jz_binding = jzon_get(jz_constant_buffer, "binding");
                    ensure(jz_binding && jz_binding->is_int);
                    pr.constant_buffers[cb_idx].binding = jz_binding->int_val;

                    let jz_fields = jzon_get(jz_constant_buffer, "fields");
                    ensure(jz_fields && jz_fields->is_array);

                    pr.constant_buffers[cb_idx].fields_num = (u32)jz_fields->num;
                    pr.constant_buffers[cb_idx].fields = mema_zero_tn(ConstantBufferField, pr.constant_buffers[cb_idx].fields_num);
                    for (u32 i = 0; i < jz_fields->size; ++i)
                        pr.constant_buffers[cb_idx].fields[i] = resource_load_parse_constant_buffer_field(jz_fields[i]);
                }
            }

            let jz_vertex_input = jzon_get(jpr.output, "vertex_input");

            if (jz_vertex_input)
            {
                ensure(jz_vertex_input->is_array);
                pr.vertex_input_num = (u32)jz_vertex_input->size;
                pr.vertex_input = mema_zero_tn(VertexInputField, pr.vertex_input_num);
                for (u32 i = 0; i < jz_vertex_input->size; ++i)
                {
                    VertexInputField* vif = &pr.vertex_input[i];
                    JzonValue jz_vif = jz_vertex_input->array_val[i];
                    ensure(jz_vif.is_table);

                    let jz_name = jzon_get(jz_vif, "name");
                    ensure(jz_name && jz_name->is_string);
                    vif->name = str_copy(jz_name->string_val);

                    let jz_type = jzon_get(jz_vif, "type");
                    ensure(jz_type && jz_type->is_string);
                    ShaderDataType sdt = shader_data_type_str_to_enum(jz_type->string_val);
                    ensure(sdt != SHADER_DATA_TYPE_INVALID);
                    vif->type = sdt;

                    let jz_vif_val = jzon_get(jz_vif, "value");
                    ensure(jz_vif_val && jz_vif_val->is_string);
                    VertexInputValue val = il_val_str_to_enum(jz_vif_val->string_val);
                    ensure(val != VERTEX_INPUT_VALUE_INVALID);
                    vif->value = val;
                }
            }

            jzon_free(&jpr.output);

            r.flag = RENDER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT;
            r.data = mema_copy_t(&pr, PipelineRenderResource);
        } break;


        case RENDER_RESOURCE_TYPE_MESH: {
            FileLoadResult flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
            check(flr.ok, "Failed loading mesh from %s", filename);
            JzonParseResult jpr = jzon_parse((char*)flr.data);
            check(jpr.ok && jpr.output.is_table, "Outer object in %s isn't a table", filename);
            memf(flr.data);

            let jz_source = jzon_get(jpr.output, "source");
            check(jz_source && jz_source->is_string, "%s doesn't contain source field", filename);

            ObjLoadResult olr = obj_load(jz_source->string_val);
            check(olr.ok, "Failed loading obj specified by %s in %s", jz_source->string_val, filename);
            jzon_free(&jpr.output);

            check(olr.ok, "Failed loading mesh from file %s", filename);

            MeshRenderResource m = {
              .mesh = olr.mesh
            };

            r.data = mema_copy_t(&m, MeshRenderResource);
        } break;

        default: error("Implement me!"); break;
    }

    RenderResourceHandle h = handle_pool_borrow(rs.resource_handle_pool, (u32)type);
    r.handle = h;

    u32 num_needed_resources = handle_index(h) + 1;
    if (num_needed_resources > rs.resources_num)
    {
        let old_num = rs.resources_num;
        let new_num = num_needed_resources ? num_needed_resources * 2 : 1;
        rs.resources = (RenderResource*)memra_zero_added(rs.resources, new_num * sizeof(RenderResource), old_num * sizeof(RenderResource));
        rs.resources_num = new_num;
    }
    rs.resources[handle_index(h)] = r;

    init_resource(h);
    return h;
}

void renderer_shutdown()
{
    info("Shutting down render");
    renderer_backend_wait_until_idle();
    
    info("Destroying all render resources");
    for (u32 i = 0; i < rs.resources_num; ++i)
    {
        let h = rs.resources[i].handle;

        if (!h)
            continue;

        destroy_resource(h);
    }

    memf(rs.resources);
    renderer_backend_shutdown();
    handle_hash_map_destroy(rs.resource_name_to_handle);
    handle_pool_destroy(rs.resource_handle_pool);
}

RenderResourceHandle renderer_create_world()
{
    RenderResourceHandle h = handle_pool_borrow(rs.resource_handle_pool, (u32)RENDER_RESOURCE_TYPE_WORLD);
    let wrr = mema_zero_t(WorldRenderResource);
    WorldObject dummy = {};
    da_push(wrr->objects, dummy);
    RenderResource r = {
        .handle = h,
        .data = wrr
    };

    u32 num_needed_resources = handle_index(h) + 1;
    if (num_needed_resources > rs.resources_num)
    {
        rs.resources = (RenderResource*)memra_zero_added(rs.resources, num_needed_resources * sizeof(RenderResource), rs.resources_num * sizeof(RenderResource));
        rs.resources_num = num_needed_resources;
    }
    rs.resources[handle_index(h)] = r;
    return h;
}

void renderer_destroy_world(RenderResourceHandle h)
{
    destroy_resource(h);
}

u32 renderer_create_object(RenderResourceHandle world, RenderResourceHandle mesh, const Vec3& pos, const Quat& rot)
{
    WorldRenderResource* w = get_resource(WorldRenderResource, world);
    Mat4 model = mat4_from_rotation_and_translation(rot, pos);

    if (da_num(w->free_object_indices) > 0)
    {
        u32 idx = da_pop(w->free_object_indices);
        w->objects[idx].mesh = mesh;
        w->objects[idx].model = model;
        return idx;
    }

    RenderWorldObjectHandle h = da_num(w->objects);

    WorldObject wo = {
        .mesh = mesh,
        .model = model
    };

    da_push(w->objects, wo);
    return h;
}

void renderer_destroy_object(RenderResourceHandle world, RenderWorldObjectHandle h)
{
    WorldRenderResource* w = get_resource(WorldRenderResource, world);
    check(w->objects[h].mesh == HANDLE_INVALID, "Trying to remove from world twice");
    w->objects[h].mesh = HANDLE_INVALID;
    da_push(w->free_object_indices, h);
}

void renderer_world_set_position_and_rotation(RenderResourceHandle world, RenderWorldObjectHandle h, const Vec3& pos, const Quat& rot)
{
    WorldRenderResource* w = get_resource(WorldRenderResource, world);
    w->objects[h].model = mat4_from_rotation_and_translation(rot, pos);
}

static void populate_constant_buffers(const PipelineRenderResource& pr, const Mat4& model_matrix, const Mat4& mvp_matrix)
{
    for (u32 cb_idx = 0; cb_idx < pr.constant_buffers_num; ++cb_idx)
    {
        ConstantBuffer* cb = pr.constant_buffers + cb_idx;

        u32 offset = 0;

        for (u32 cbf_idx = 0; cbf_idx < cb->fields_num; ++cbf_idx)
        {
            ConstantBufferField* cbf = cb->fields + cbf_idx;

            switch(cbf->auto_value)
            {
                case CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL:
                    renderer_backend_update_constant_buffer(*pr.backend_state, cb->binding, &model_matrix, sizeof(model_matrix), offset);
                    break;

                case CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL_VIEW_PROJECTION:
                    renderer_backend_update_constant_buffer(*pr.backend_state, cb->binding, &mvp_matrix, sizeof(mvp_matrix), offset);
                    break;

                default: break;
            }

            offset += shader_data_type_size(cbf->type);
        }

    }
}

void renderer_begin_frame(RenderResourceHandle pipeline_handle)
{
    PipelineRenderResource* pipeline = get_resource(PipelineRenderResource, pipeline_handle);
    (void)pipeline;
    //
    //populate_constant_buffers(*pipeline, model, mvp_matrix);

    renderer_backend_begin_frame(pipeline->backend_state);
}

void renderer_draw(RenderResourceHandle pipeline_handle, RenderResourceHandle mesh_handle, const Mat4& model, const Vec3& cam_pos, const Quat& cam_rot)
{
    Mat4 camera_matrix = mat4_from_rotation_and_translation(cam_rot, cam_pos);
    Mat4 view_matrix = inverse(camera_matrix);

    Vec2u size = renderer_backend_get_size();
    Mat4 proj_matrix = mat4_create_projection_matrix(size.x, size.y);
    Mat4 mvp_matrix = model * view_matrix * proj_matrix;

    PipelineRenderResource* pipeline = get_resource(PipelineRenderResource, pipeline_handle);

    renderer_backend_draw(pipeline->backend_state, get_resource(MeshRenderResource, mesh_handle)->backend_state, mvp_matrix, model);
}

void renderer_draw_world(RenderResourceHandle pipeline_handle, RenderResourceHandle world_handle, const Vec3& cam_pos, const Quat& cam_rot)
{
    WorldRenderResource* w = get_resource(WorldRenderResource, world_handle);

    for (u32 i = 0; i < da_num(w->objects); ++i)
    {
        WorldObject* obj = w->objects + i;

        if (obj->mesh == HANDLE_INVALID)
            continue;

        renderer_draw(pipeline_handle, obj->mesh, obj->model, cam_pos, cam_rot);
    }
}

void renderer_present()
{
    renderer_backend_present();
}

void renderer_surface_resized(u32 w, u32 h)
{
    info("Render resizing to %d x %d", w, h);
    renderer_backend_wait_until_idle();
    renderer_backend_surface_resized(w, h);

    for (u32 i = 0; i < rs.resources_num; ++i)
    {
        RenderResource* rr = rs.resources + i;

        if (rr->flag & RENDER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT)
            reinit_resource(rr->handle);
    }
    renderer_backend_wait_until_idle();
}

void renderer_debug_draw_mesh(const Vec3* vertices, u32 vertices_num, const Color& c, const Vec3& cam_pos, const Quat& cam_rot)
{
    Mat4 camera_matrix = mat4_from_rotation_and_translation(cam_rot, cam_pos);
    Mat4 view_matrix = inverse(camera_matrix);

    Vec2u size = renderer_backend_get_size();
    Mat4 proj_matrix = mat4_create_projection_matrix(size.x, size.y);
    Mat4 vp_matrix = view_matrix * proj_matrix;

    renderer_backend_debug_draw_mesh(get_resource(PipelineRenderResource, rs.debug_draw_pipeline)->backend_state, vertices, vertices_num, c, vp_matrix);
}