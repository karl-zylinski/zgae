#pragma once
#include "jzon_types.h"

jzon_parse_result_t jzon_parse(const char* input);
void jzon_free(jzon_value_t* val);
const jzon_value_t* jzon_get(const jzon_value_t* object, const char* key);