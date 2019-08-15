#include "pipeline.h"
#include "debug.h"
#include "file.h"
#include "jzon.h"
#include "memory.h"
#include "shader.h"
#include "renderer.h"

renderer_resource_handle_t pipeline_load(renderer_state_t* rs, const char* filename)
{
    info("Loading pipeline from %s", filename);
    pipeline_intermediate_t pi = {};
    #define ensure(expr) if (!(expr)) return HANDLE_INVALID
    char* fd;
    size_t fs;
    bool file_load_ok = file_load_str(filename, &fd, &fs);
    ensure(file_load_ok);
    jzon_value_t parsed;
    int parse_res = jzon_parse(fd, &parsed);
    ensure(parse_res && parsed.is_table);
    memf(fd);

    jzon_value_t* jz_ss_arr = jzon_get(&parsed, "shader_stages");
    ensure(jz_ss_arr && jz_ss_arr->is_array);
    pi.shader_stages_num = (unsigned)jz_ss_arr->size;
    pi.shader_stages = mema_zero(sizeof(shader_intermediate_t) * pi.shader_stages_num);
    
    for (uint32_t i = 0; i < jz_ss_arr->size; ++i)
    {
        jzon_value_t* jz_ss_item = jz_ss_arr->array_val + i;
        ensure(jz_ss_item->is_string);
        pi.shader_stages[i] = shader_load(rs, jz_ss_item->string_val);
        ensure(pi.shader_stages[i] != HANDLE_INVALID);
    }

    renderer_resource_handle_t p = renderer_load_pipeline(rs, &pi);
    ensure(p != HANDLE_INVALID);
    return p;
}