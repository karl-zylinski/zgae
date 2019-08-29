#pragma once
#include "jzon_types.h"

JzonParseResult jzon_parse(char* input);
void jzon_free(JzonValue* val);
const JzonValue* jzon_get(const JzonValue* table, char* key);