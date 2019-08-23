#include "shader_resource.h"
#include "jzon.h"
#include "file.h"
#include "str.h"
#include "renderer.h"
#include "memory.h"
#include "debug.h"

static ShaderType type_str_to_enum(const char* str)
{
    if (str_eql(str, "vertex"))
        return SHADER_TYPE_VERTEX;

    if (str_eql(str, "fragment"))
        return SHADER_TYPE_FRAGMENT;

    return SHADER_TYPE_INVALID;
}


ShaderResource shader_resource_load(const char* filename)
{
    info("Loading shader from %s", filename);
    #define format_check(cond, msg, ...) (check(cond, "When parsing shader %s: %s", filename, msg, ##__VA_ARGS__))
    ShaderResource sr = {};
    FileLoadResult shader_flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
    format_check(shader_flr.ok, "File missing");
    JzonParseResult jpr = jzon_parse(shader_flr.data);
    format_check(jpr.ok && jpr.output.is_table, "Malformed shader");
    memf(shader_flr.data);
    
    const JzonValue* jz_type = jzon_get(&jpr.output, "type");
    format_check(jz_type && jz_type->is_string, "type not a string or missing");
    ShaderType st = type_str_to_enum(jz_type->string_val);
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
    return sr;
}

u32 shader_data_type_size(ShaderDataType t)
{
    switch (t)
    {
        case SHADER_DATA_TYPE_MAT4: return 64;
        case SHADER_DATA_TYPE_VEC2: return 8;
        case SHADER_DATA_TYPE_VEC3: return 12;
        case SHADER_DATA_TYPE_VEC4: return 16;
        case SHADER_DATA_TYPE_INVALID: break;
    }

    error("Trying to get size of invalid ShaderDataType");
    return 0;
}