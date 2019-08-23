#pragma once

fwd_handle(ResourceHandle);
fwd_struct(Resource);

void resource_store_init();
ResourceHandle resource_load(const char* filename);
const Resource* resource_lookup(ResourceHandle h);