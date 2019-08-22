#pragma once
#include "pipeline_types.h"

fwd_struct(RendererState);

RendererResourceHandle pipeline_load(RendererState* rs, const char* filename);