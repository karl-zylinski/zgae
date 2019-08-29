#pragma once
#include "file_types.h"

FileLoadResult file_load(const char* filename, FileLoadMode mode = FILE_LOAD_MODE_RAW);