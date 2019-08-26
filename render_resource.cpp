#include "render_resource.h"
#include "render_resource_types.h"
#include "debug.h"

u32 shader_data_type_size(ShaderDataType t)
{
    switch (t)
    {
        case ShaderDataType::Mat4: return 64;
        case ShaderDataType::Vec2: return 8;
        case ShaderDataType::Vec3: return 12;
        case ShaderDataType::Vec4: return 16;
        case ShaderDataType::Invalid: break;
    }

    error("Trying to get size of invalid ShaderDataType");
    return 0;
}