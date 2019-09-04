#include "render_resource.h"
#include "debug.h"

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