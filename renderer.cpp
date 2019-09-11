#include "renderer.h"
#include "renderer_backend.h"
#include "memory.h"
#include "dynamic_array.h"
#include "log.h"
#include "render_resource.h"
#include "str.h"
#include "idx_hash_map.h"
#include "path.h"
#include "file.h"
#include "jzon.h"
#include "obj_loader.h"

struct Shader
{
    u32 idx;
    i64 namehash;
    char* source;
    u64 source_size;
    ShaderType type;
    RenderBackendShader* backend_state;
    ConstantBufferField* push_constant_fields;
    u32 push_constant_fields_num;
};

struct Pipeline
{
    u32 idx;
    i64 namehash;
    u32* shader_stages;
    ConstantBuffer* constant_buffers;
    VertexInputField* vertex_input;
    u32 shader_stages_num;
    u32 vertex_input_num;
    u32 constant_buffers_num;
    RenderBackendPipeline* backend_state;
    PrimitiveTopology primitive_topology;
    bool depth_test;
};

struct RenderMesh
{
    u32 idx;
    i64 namehash;
    Mesh mesh;
    RenderBackendMesh* backend_state;
};

struct RenderObject
{
    u32 idx;
    Mat4 model;
    u32 mesh_idx;
};

struct RenderWorld
{
    RenderObject* objects; // dynamic
    u32* objects_free_idx; // dynamic
};

struct Renderer
{
    RenderMesh* meshes; // dynamic
    u32* meshes_free_idx; // dynamic
    IdxHashMap* meshes_lut;
    Pipeline* pipelines; // dynamic
    u32* pipelines_free_idx; // dynamic
    IdxHashMap* pipelines_lut;
    Shader* shaders; // dynamic
    u32* shaders_free_idx; // dynamic
    IdxHashMap* shaders_lut;
    u32 debug_draw_traingles_pipeline_idx;
    u32 debug_draw_line_pipeline_idx;
};

static Renderer rs = {};
static bool inited = false;

void renderer_init(WindowType window_type, const GenericWindowInfo& window_info)
{
    check(!inited, "Trying to init renderer twice!");
    inited = true;
    da_push(rs.meshes, RenderMesh{}); // reserve zero
    da_push(rs.pipelines, Pipeline{}); // reserve zero
    da_push(rs.shaders, Shader{}); // reserve zero
    rs.meshes_lut = idx_hash_map_create();
    rs.pipelines_lut = idx_hash_map_create();
    rs.shaders_lut = idx_hash_map_create();
    renderer_backend_init(window_type, window_info);
    rs.debug_draw_traingles_pipeline_idx = renderer_load_pipeline("pipeline_debug_draw_triangles.pipeline");
    rs.debug_draw_line_pipeline_idx = renderer_load_pipeline("pipeline_debug_draw_line.pipeline");
}

static void init_pipeline(u32 pipeline_idx)
{
    let p = rs.pipelines + pipeline_idx;

    RenderBackendShader** backend_shader_stages = mema_tn(RenderBackendShader*, p->shader_stages_num);
    ShaderType* backend_shader_types = mema_tn(ShaderType, p->shader_stages_num);
    u32 push_constants_num = 0;
    u32* push_constants_sizes = NULL;
    ShaderType* push_constants_shader_types = NULL;

    for (u32 shdr_idx = 0; shdr_idx < p->shader_stages_num; ++shdr_idx)
    {
        let s = rs.shaders + p->shader_stages[shdr_idx];
        backend_shader_stages[shdr_idx] = s->backend_state;
        backend_shader_types[shdr_idx] = s->type;

        if (s->push_constant_fields_num > 0)
        {
            u32 cb_size = 0;

            for (u32 pc_field_idx = 0; pc_field_idx < s->push_constant_fields_num; ++pc_field_idx)
                cb_size += shader_data_type_size(s->push_constant_fields[pc_field_idx].type);

            da_push(push_constants_shader_types, s->type);
            da_push(push_constants_sizes, cb_size);
            ++push_constants_num;
        }
    }

    ShaderDataType* vertex_input_types = mema_tn(ShaderDataType, p->vertex_input_num);
    for (u32 vi_idx = 0; vi_idx < p->vertex_input_num; ++vi_idx)
        vertex_input_types[vi_idx] = p->vertex_input[vi_idx].type;

    u32* constant_buffer_sizes = mema_tn(u32, p->constant_buffers_num);
    u32* constant_buffer_binding_indices = mema_tn(u32, p->constant_buffers_num);

    for (u32 cb_idx = 0; cb_idx < p->constant_buffers_num; ++cb_idx)
    {
        u32 s = 0;

        for (u32 cb_field_idx = 0; cb_field_idx < p->constant_buffers[cb_idx].fields_num; ++cb_field_idx)
            s += shader_data_type_size(p->constant_buffers[cb_idx].fields[cb_field_idx].type);

        constant_buffer_sizes[cb_idx] = s;
        constant_buffer_binding_indices[cb_idx] = p->constant_buffers[cb_idx].binding;
    }

    p->backend_state = renderer_backend_create_pipeline(
        backend_shader_stages, backend_shader_types, p->shader_stages_num,
        vertex_input_types, p->vertex_input_num,
        constant_buffer_sizes, constant_buffer_binding_indices, p->constant_buffers_num,
        push_constants_sizes, push_constants_shader_types, push_constants_num,
        p->primitive_topology, p->depth_test);

    da_free(push_constants_sizes);
    da_free(push_constants_shader_types);
    memf(constant_buffer_binding_indices);
    memf(constant_buffer_sizes);
    memf(vertex_input_types);
    memf(backend_shader_types);
    memf(backend_shader_stages);
}

static void deinit_pipeline(u32 pipeline_idx)
{
    let p = rs.pipelines + pipeline_idx;
    renderer_backend_destroy_pipeline(p->backend_state);
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

static PrimitiveTopology primitive_topology_str_to_enum(const char* str)
{
    if (str_eql(str, "triangle_list"))
        return PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    if (str_eql(str, "triangle_strip"))
        return PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    if (str_eql(str, "line_strip"))
        return PRIMITIVE_TOPOLOGY_LINE_STRIP;

    error("Unknown primtive topology");
}

u32 renderer_load_mesh(const char* filename)
{
    let filename_hash = str_hash(filename);
    let existing = idx_hash_map_get(rs.meshes_lut, filename_hash);

    if (existing)
        return existing;

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

    let idx = da_num(rs.meshes_free_idx) > 0 ? da_pop(rs.meshes_free_idx) : da_num(rs.meshes);

    RenderMesh m = {
        .idx = idx,
        .namehash = filename_hash,
        .mesh = olr.mesh,
        .backend_state = renderer_backend_create_mesh(&olr.mesh),
    };

    da_insert(rs.meshes, m, idx);
    idx_hash_map_add(rs.meshes_lut, filename_hash, idx);
    return idx;
}

void renderer_destroy_mesh(u32 mesh_idx)
{
    let m = rs.meshes + mesh_idx;
    renderer_backend_destroy_mesh(m->backend_state);
    memf(m->mesh.vertices);
    memf(m->mesh.indices);
    idx_hash_map_remove(rs.meshes_lut, m->namehash);
    memzero(m, sizeof(RenderMesh));
    da_push(rs.meshes_free_idx, mesh_idx);
}

static u32 load_shader(const char* filename)
{
    let filename_hash = str_hash(filename);
    let existing = idx_hash_map_get(rs.shaders_lut, filename_hash);

    if (existing)
        return existing;

    Shader s = {};
    FileLoadResult shader_flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
    check(shader_flr.ok, "File missing");
    JzonParseResult jpr = jzon_parse((char*)shader_flr.data);
    check(jpr.ok && jpr.output.is_table, "Malformed shader");
    memf(shader_flr.data);
    
    let jz_type = jzon_get(jpr.output, "type");
    check(jz_type && jz_type->is_string, "type not a string or missing");
    ShaderType st = shader_type_str_to_enum(jz_type->string_val);
    check(st != SHADER_TYPE_INVALID, "type isn't an allowed value");
    s.type = st;

    let jz_source = jzon_get(jpr.output, "source");
    check(jz_source && jz_source->is_string, "source missing or not a string");

    let jz_push_constant = jzon_get(jpr.output, "push_constant");

    if (jz_push_constant)
    {
        check(jz_push_constant->is_array, "push_constant isn't array");
        s.push_constant_fields_num  = (u32)jz_push_constant->num;
        s.push_constant_fields = mema_zero_tn(ConstantBufferField, s.push_constant_fields_num);

        for (u32 i = 0; i < s.push_constant_fields_num; ++i)
            s.push_constant_fields[i] = resource_load_parse_constant_buffer_field(jz_push_constant->array_val[i]);
    }

    FileLoadResult source_flr = file_load(jz_source->string_val);
    check(source_flr.ok, "failed opening shader source %s", jz_source->string_val);
    s.source = (char*)mema_copy(source_flr.data, source_flr.data_size);
    s.source_size = source_flr.data_size;
    memf(source_flr.data);
    jzon_free(&jpr.output);

    let idx = da_num(rs.shaders_free_idx) > 0 ? da_pop(rs.shaders_free_idx) : da_num(rs.shaders);
    s.idx = idx;
    s.namehash = filename_hash;
    s.backend_state = renderer_backend_create_shader(s.source, s.source_size);
    da_insert(rs.shaders, s, idx);
    idx_hash_map_add(rs.shaders_lut, filename_hash, idx);
    return idx;
}

static void destroy_shader(u32 shader_idx)
{
    let s = rs.shaders + shader_idx;
    renderer_backend_destroy_shader(s->backend_state);
    for (u32 i = 0; i < s->push_constant_fields_num; ++i)
            memf(s->push_constant_fields[i].name);
    memf(s->push_constant_fields);
    memf(s->source);
    idx_hash_map_remove(rs.shaders_lut, s->namehash);
    memzero(s, sizeof(Shader));
    da_push(rs.shaders_free_idx, shader_idx);
}

u32 renderer_load_pipeline(const char* filename)
{
    let filename_hash = str_hash(filename);
    let existing = idx_hash_map_get(rs.pipelines_lut, filename_hash);

    if (existing)
        return existing;

    Pipeline p = {};
    #define ensure(expr) if (!(expr)) error("Error in pipeline resource load");
    FileLoadResult flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
    ensure(flr.ok);
    JzonParseResult jpr = jzon_parse((char*)flr.data);
    ensure(jpr.ok && jpr.output.is_table);
    memf(flr.data);

    let jz_shader_stages = jzon_get(jpr.output, "shader_stages");
    ensure(jz_shader_stages && jz_shader_stages->is_array);
    p.shader_stages_num = jz_shader_stages->size;
    p.shader_stages = mema_zero_tn(u32, p.shader_stages_num);
    
    for (u32 shdr_idx = 0; shdr_idx < jz_shader_stages->size; ++shdr_idx)
    {
        let jz_shader_stage = jz_shader_stages->array_val + shdr_idx;
        ensure(jz_shader_stage->is_string);
        p.shader_stages[shdr_idx] = load_shader(jz_shader_stage->string_val);
    }

    let jz_constant_buffers = jzon_get(jpr.output, "constant_buffers");

    if (jz_constant_buffers)
    {
        ensure(jz_constant_buffers->is_array);

        p.constant_buffers_num = jz_constant_buffers->num;
        p.constant_buffers = mema_zero_tn(ConstantBuffer, p.constant_buffers_num);

        for (u32 cb_idx = 0; cb_idx < p.constant_buffers_num; ++cb_idx)
        {
            JzonValue jz_constant_buffer = jz_constant_buffers->array_val[cb_idx];
            ensure(jz_constant_buffer.is_table);

            let jz_binding = jzon_get(jz_constant_buffer, "binding");
            ensure(jz_binding && jz_binding->is_int);
            p.constant_buffers[cb_idx].binding = jz_binding->int_val;

            let jz_fields = jzon_get(jz_constant_buffer, "fields");
            ensure(jz_fields && jz_fields->is_array);

            p.constant_buffers[cb_idx].fields_num = (u32)jz_fields->num;
            p.constant_buffers[cb_idx].fields = mema_zero_tn(ConstantBufferField, p.constant_buffers[cb_idx].fields_num);
            for (u32 i = 0; i < jz_fields->size; ++i)
                p.constant_buffers[cb_idx].fields[i] = resource_load_parse_constant_buffer_field(jz_fields[i]);
        }
    }

    let jz_vertex_input = jzon_get(jpr.output, "vertex_input");

    if (jz_vertex_input)
    {
        ensure(jz_vertex_input->is_array);
        p.vertex_input_num = (u32)jz_vertex_input->size;
        p.vertex_input = mema_zero_tn(VertexInputField, p.vertex_input_num);
        for (u32 i = 0; i < jz_vertex_input->size; ++i)
        {
            VertexInputField* vif = &p.vertex_input[i];
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

    let jz_prim_topo = jzon_get(jpr.output, "primitive_topology");
    check(jz_prim_topo && jz_prim_topo->is_string, "primitive_topology missing or not string");
    p.primitive_topology = primitive_topology_str_to_enum(jz_prim_topo->string_val);

    let jz_depth_test = jzon_get(jpr.output, "depth_test");
    check(jz_depth_test == NULL || jz_depth_test->is_bool, "depth_test must be a bool");
    p.depth_test = jz_depth_test == NULL || jz_depth_test->bool_val;

    jzon_free(&jpr.output);

    let idx = da_num(rs.pipelines_free_idx) > 0 ? da_pop(rs.pipelines_free_idx) : da_num(rs.pipelines);
    p.idx = idx;
    p.namehash = filename_hash;
    da_insert(rs.pipelines, p, idx);
    idx_hash_map_add(rs.pipelines_lut, filename_hash, idx);
    init_pipeline(idx);
    return idx;
}

void renderer_destroy_pipeline(u32 pipeline_idx)
{
    deinit_pipeline(pipeline_idx);
    let p = rs.pipelines + pipeline_idx;
    memf(p->shader_stages);

    for (u32 i = 0; i < p->constant_buffers_num; ++i)
    {
        for (u32 j = 0; j < p->constant_buffers[i].fields_num; ++j)
            memf(p->constant_buffers[i].fields[j].name);

        memf(p->constant_buffers[i].fields);
    }

    memf(p->constant_buffers);

    for (u32 i = 0; i < p->vertex_input_num; ++i)
        memf(p->vertex_input[i].name);
    
    memf(p->vertex_input);
    memzero(p, sizeof(Pipeline));
    idx_hash_map_remove(rs.pipelines_lut, p->namehash);
    da_push(rs.pipelines_free_idx, pipeline_idx);
}

void renderer_shutdown()
{
    info("Shutting down render");
    renderer_backend_wait_until_idle();
    
    for (u32 i = 1; i < da_num(rs.meshes); ++i)
        if (rs.meshes[i].idx == i)
            renderer_destroy_mesh(i);

    da_free(rs.meshes);
    da_free(rs.meshes_free_idx);

    for (u32 i = 1; i < da_num(rs.shaders); ++i)
        if (rs.shaders[i].idx == i)
            destroy_shader(i);

    da_free(rs.shaders);
    da_free(rs.shaders_free_idx);

    for (u32 i = 1; i < da_num(rs.pipelines); ++i)
        if (rs.pipelines[i].idx == i)
            renderer_destroy_pipeline(i);

    da_free(rs.pipelines);
    da_free(rs.pipelines_free_idx);

    idx_hash_map_destroy(rs.meshes_lut);
    idx_hash_map_destroy(rs.pipelines_lut);
    idx_hash_map_destroy(rs.shaders_lut);

    renderer_backend_shutdown();
}

RenderWorld* renderer_create_world()
{
    let w = mema_zero_t(RenderWorld);
    da_push(w->objects, RenderObject{}); // zero pos dummy
    return w;
}

void renderer_destroy_world(RenderWorld* w)
{
    da_free(w->objects);
    memf(w);
}

u32 renderer_create_object(RenderWorld* w, u32 mesh_idx, const Vec3& pos, const Quat& rot)
{
    Mat4 model = mat4_from_rotation_and_translation(rot, pos);
    let idx = da_num(w->objects_free_idx) > 0 ? da_pop(w->objects_free_idx) : da_num(w->objects);

    RenderObject wo = {
        .idx = idx,
        .mesh_idx = mesh_idx,
        .model = model
    };

    da_insert(w->objects, wo, idx);
    return idx;
}

void renderer_destroy_object(RenderWorld* w, u32 object_idx)
{
    let o = w->objects + object_idx;
    check(o->idx, "Trying to remove from world twice");
    memzero(o, sizeof(RenderObject));
    da_push(w->objects_free_idx, object_idx);
}

void renderer_world_set_position_and_rotation(RenderWorld* w, u32 object_idx, const Vec3& pos, const Quat& rot)
{
    w->objects[object_idx].model = mat4_from_rotation_and_translation(rot, pos);
}

static void populate_constant_buffers(const Pipeline& p, const Mat4& model_matrix, const Mat4& mvp_matrix)
{
    for (u32 cb_idx = 0; cb_idx < p.constant_buffers_num; ++cb_idx)
    {
        ConstantBuffer* cb = p.constant_buffers + cb_idx;

        u32 offset = 0;

        for (u32 cbf_idx = 0; cbf_idx < cb->fields_num; ++cbf_idx)
        {
            ConstantBufferField* cbf = cb->fields + cbf_idx;

            switch(cbf->auto_value)
            {
                case CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL:
                    renderer_backend_update_constant_buffer(*p.backend_state, cb->binding, &model_matrix, sizeof(model_matrix), offset);
                    break;

                case CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL_VIEW_PROJECTION:
                    renderer_backend_update_constant_buffer(*p.backend_state, cb->binding, &mvp_matrix, sizeof(mvp_matrix), offset);
                    break;

                default: break;
            }

            offset += shader_data_type_size(cbf->type);
        }

    }
}

void renderer_begin_frame(u32 pipeline_idx)
{
    let pipeline = rs.pipelines + pipeline_idx;
    //
    //populate_constant_buffers(*pipeline, model, mvp_matrix);

    renderer_backend_begin_frame(pipeline->backend_state);
}

void renderer_draw(u32 pipeline_idx, u32 mesh_idx, const Mat4& model, const Vec3& cam_pos, const Quat& cam_rot)
{
    Mat4 camera_matrix = mat4_from_rotation_and_translation(cam_rot, cam_pos);
    Mat4 view_matrix = inverse(camera_matrix);

    Vec2u size = renderer_backend_get_size();
    Mat4 proj_matrix = mat4_create_projection_matrix(size.x, size.y);
    Mat4 mvp_matrix = model * view_matrix * proj_matrix;

    let pipeline = rs.pipelines + pipeline_idx;

    renderer_backend_draw(pipeline->backend_state, rs.meshes[mesh_idx].backend_state, mvp_matrix, model);
}

void renderer_draw_world(u32 pipeline_idx, RenderWorld* w, const Vec3& cam_pos, const Quat& cam_rot)
{
    for (u32 i = 0; i < da_num(w->objects); ++i)
    {
        let obj = w->objects + i;

        if (!obj->idx)
            continue;

        renderer_draw(pipeline_idx, obj->mesh_idx, obj->model, cam_pos, cam_rot);
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

    // Pipelines are usually size-dependent.
    for (u32 i = 0; i < da_num(rs.pipelines); ++i)
    {
        if (!rs.pipelines[i].idx)
            continue;

        deinit_pipeline(i);
        init_pipeline(i);
    }

    renderer_backend_wait_until_idle();
}

static u32 get_pipeline_for_topology(PrimitiveTopology pt)
{
    switch(pt)
    {
        case PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: return rs.debug_draw_traingles_pipeline_idx;
        case PRIMITIVE_TOPOLOGY_LINE_STRIP: return rs.debug_draw_line_pipeline_idx;
        default: error("Debug drawing not implemented for given PrimitiveTopology");
    }
}

void renderer_debug_draw(const Vec3* vertices, u32 vertices_num, const Vec4* colors, PrimitiveTopology pt, const Vec3& cam_pos, const Quat& cam_rot)
{
    Mat4 camera_matrix = mat4_from_rotation_and_translation(cam_rot, cam_pos);
    Mat4 view_matrix = inverse(camera_matrix);

    Vec2u size = renderer_backend_get_size();
    Mat4 proj_matrix = mat4_create_projection_matrix(size.x, size.y);
    Mat4 vp_matrix = view_matrix * proj_matrix;

    Vec4 white = {1,1,1,1};
    SimpleVertex* mesh = mema_tn(SimpleVertex, vertices_num);

    for (u32 i = 0; i < vertices_num; ++i)
    {
        mesh[i].position = vertices[i];
        mesh[i].color = colors == NULL ? white : colors[i];
    }

    renderer_backend_debug_draw(rs.pipelines[get_pipeline_for_topology(pt)].backend_state, mesh, vertices_num, vp_matrix);
    memf(mesh);
}