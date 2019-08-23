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

typedef struct ResourceFilenameMapping
{
    hash64 name_hash;
    ResourceHandle handle;
} ResourceFilenameMapping;

ResourceFilenameMapping* g_mapping = NULL;

static sizet find_mapping_insertion_idx(hash64 name_hash)
{
    if (array_num(g_mapping) == 0)
        return 0;

    for (sizet i = 0; i < array_num(g_mapping); ++i)
    {
        if (g_mapping[i].name_hash > name_hash)
            return i;
    }

    return array_num(g_mapping);
}

static ResourceHandle mapping_get(hash64 name_hash)
{
    if (array_num(g_mapping) == 0)
        return HANDLE_INVALID;

    sizet mz = array_num(g_mapping);
    sizet first = 0;
    sizet last = mz - 1;
    sizet middle = (first + last) / 2;

    while (first <= last)
    {
        if (g_mapping[middle].name_hash < name_hash)
            first = middle + 1;
        else if (g_mapping[middle].name_hash == name_hash)
            return g_mapping[middle].handle;
        else
        {
            if (middle == 0)
                break;

            last = middle - 1;
        }

        middle = (first + last) / 2;
    }

    return HANDLE_INVALID;
}

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
    hash64 name_hash = str_hash(filename);
    ResourceHandle existing = mapping_get(name_hash);

    if (existing != HANDLE_INVALID)
        return existing;

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
    ResourceFilenameMapping rfm = {.handle = h, .name_hash = name_hash};
    array_insert(g_mapping, rfm, find_mapping_insertion_idx(name_hash));
    return h;
}

const Resource* resource_lookup(ResourceHandle h)
{
    return da_resources + handle_index(h);
}