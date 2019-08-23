#pragma once
#include "shader_resource_types.h"

ShaderResource shader_resource_load(const char* filename);
u32 shader_data_type_size(ShaderDataType t);