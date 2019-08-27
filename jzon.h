#pragma once
#include "jzon_types.h"

JzonParseResult jzon_parse(char* input);
void jzon_free(mut JzonValue* val);
JzonValue* jzon_get(JzonValue* object, char* key);