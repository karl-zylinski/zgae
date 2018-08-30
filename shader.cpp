#include "shader.h"
#include "jzon.h"
#include "file.h"

RRHandle shader_load(Renderer* r, const char* filename)
{
    LoadedFile shader_file = file_load(filename, FileEnding::Zero);

    if (!shader_file.valid)
        return {InvalidHandle};

    JzonParseResult jpr = jzon_parse((const char*)shader_file.file.data);
    (void)jpr;
    return {0};
}
