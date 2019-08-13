#pragma once

#define renderer_resource_t uint32
struct renderer_state;

enum shader_data_type
{
    SHADER_DATA_TYPE_INVALID,
    SHADER_DATA_TYPE_MAT4,
    SHADER_DATA_TYPE_VEC2,
    SHADER_DATA_TYPE_VEC3,
    SHADER_DATA_TYPE_VEC4,
};

enum shader_constant_buffer_auto_value
{
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_NONE,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_PROJECTION,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_VIEW_PROJECTION,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL_VIEW_PROJECTION
};

struct shader_constant_buffer_item
{
    char* name;
    enum shader_data_type type;
    enum shader_constant_buffer_auto_value auto_value;
};

enum shader_input_layout_value
{
    SHADER_INPUT_LAYOUT_VALUE_INVALID,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_POSITION,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_NORMAL,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_TEXCOORD,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_COLOR
};

struct shader_input_layout_item
{
    char* name;
    enum shader_data_type type;
    enum shader_input_layout_value value;
};

struct shader_intermediate
{
    char* source;
    uint64 source_size;
    struct shader_constant_buffer_item* constant_buffer;
    struct shader_input_layout_item* input_layout;
    uint32 constant_buffer_size;
    uint32 input_layout_size;
};

renderer_resource_t shader_load(struct renderer_state* rs, const char* filename);
uint32 shader_data_type_size(enum shader_data_type type);