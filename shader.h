#pragma once

#define renderer_resource_t uint32_t
fwd_struct(renderer_state_t);

typedef enum shader_data_type_t
{
    SHADER_DATA_TYPE_INVALID,
    SHADER_DATA_TYPE_MAT4,
    SHADER_DATA_TYPE_VEC2,
    SHADER_DATA_TYPE_VEC3,
    SHADER_DATA_TYPE_VEC4,
} shader_data_type_t;

typedef enum shader_constant_buffer_auto_value_t
{
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_NONE,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_PROJECTION,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_VIEW_PROJECTION,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL_VIEW_PROJECTION
} shader_constant_buffer_auto_value_t;

typedef struct shader_constant_buffer_item_t
{
    char* name;
    shader_data_type_t type;
    shader_constant_buffer_auto_value_t auto_value;
} shader_constant_buffer_item_t;

typedef enum shader_input_layout_value_t
{
    SHADER_INPUT_LAYOUT_VALUE_INVALID,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_POSITION,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_NORMAL,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_TEXCOORD,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_COLOR
} shader_input_layout_value_t;

typedef struct shader_input_layout_item_t
{
    char* name;
    shader_data_type_t type;
    shader_input_layout_value_t value;
} shader_input_layout_item_t;

typedef struct shader_intermediate_t
{
    char* source;
    uint64_t source_size;
    shader_constant_buffer_item_t* constant_buffer;
    shader_input_layout_item_t* input_layout;
    uint32_t constant_buffer_size;
    uint32_t input_layout_size;
} shader_intermediate_t;

renderer_resource_t shader_load(renderer_state_t* rs, const char* filename);
uint32_t shader_data_type_size(shader_data_type_t t);