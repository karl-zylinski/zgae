#include "resource_store.h"
#include "resource_types.h"
#include "handle_pool.h"
#include "shader_resource.h"
#include "pipeline_resource.h"
#include "path.h"
#include "str.h"
#include "debug.h"
#include "array.h"

static HandlePool* g_hp = NULL;
static Resource* da_resources = NULL;

static const char* resource_type_names[] = {
    "invalid",
    "shader",
    "pipeline"
};

static ResourceType resource_type_from_str(const char* str)
{
    i32 idx = str_eql_arr(str, resource_type_names, arrnum(resource_type_names));
    check(idx > 0 && idx < RESOURCE_TYPE_NUM, "Invalid resource type");
    return idx;
}

void resource_store_init()
{
    check(g_hp == NULL, "resource_store_init probably run twice");

    g_hp = handle_pool_create();

    for (ResourceType t = 1; t < RESOURCE_TYPE_NUM; ++t)
        handle_pool_set_type(g_hp, t, resource_type_names[t]);
}

ResourceHandle resource_load(const char* filename)
{
    const char* ext = path_ext(filename);
    ResourceType type = resource_type_from_str(ext);
    Resource r = { .type = type };

    switch(type)
    {
        case RESOURCE_TYPE_SHADER:
            r.shader = shader_resource_load(filename); break;

        case RESOURCE_TYPE_PIPELINE:
            r.pipeline = pipeline_resource_load(filename); break;

        default: error("Implement me!"); break;
    }

    ResourceHandle h = handle_pool_reserve(g_hp, r.type);
    r.handle = h;
    array_fill_and_set(da_resources, handle_index(h), r);
    return h;
}

const Resource* resource_lookup(ResourceHandle h)
{
    return da_resources + handle_index(h);
}