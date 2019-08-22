#pragma once
#include "jzon_types.h"

JzonParseResult jzon_parse(const char* input);
void jzon_free(JzonValue* val);
const JzonValue* jzon_get(const JzonValue* object, const char* key);