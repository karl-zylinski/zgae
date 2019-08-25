#pragma once

fwd_handle(ResourceHandle);
fwd_struct(Resource);
fwd_struct(RendererState);

void resource_store_init(RendererState* rs);
void resource_store_destroy();

ResourceHandle resource_load(const char* filename);
const Resource* resource_lookup(ResourceHandle h);
void resource_destroy(ResourceHandle h);
