#include "pipeline.h"
#include "debug.h"
#include "file.h"
#include "jzon.h"
#include "memory.h"
#include "shader.h"
#include "renderer.h"

RendererResourceHandle pipeline_load(RendererState* rs, const char* filename)
{
    info("Loading pipeline from %s", filename);
    PipelineIntermediate pi = {};
    #define ensure(expr) if (!(expr)) return HANDLE_INVALID
    FileLoadResult flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
    ensure(flr.ok);
    JzonParseResult jpr = jzon_parse(flr.data);
    ensure(jpr.ok && jpr.output.is_table);
    memf(flr.data);

    const JzonValue* jz_ss_arr = jzon_get(&jpr.output, "shader_stages");
    ensure(jz_ss_arr && jz_ss_arr->is_array);
    pi.shader_stages_num = (unsigned)jz_ss_arr->size;
    pi.shader_stages = mema_zero(sizeof(ShaderIntermediate) * pi.shader_stages_num);
    
    for (u32 i = 0; i < jz_ss_arr->size; ++i)
    {
        JzonValue* jz_ss_item = jz_ss_arr->array_val + i;
        ensure(jz_ss_item->is_string);
        pi.shader_stages[i] = shader_load(rs, jz_ss_item->string_val);
        ensure(pi.shader_stages[i] != HANDLE_INVALID);
    }

    jzon_free(&jpr.output);
    RendererResourceHandle p = renderer_load_pipeline(rs, &pi);
    ensure(p != HANDLE_INVALID);
    memf(pi.shader_stages);
    return p;
}