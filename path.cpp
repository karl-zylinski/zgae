#include "path.h"
#include "debug.h"

const char* path_ext(const char* p)
{
    const char* pp = p;
    const char* ext_start = NULL;

    while(*pp)
    {
        if((*pp) == '.')
            ext_start = pp + 1;

        ++pp;
    }

    check(ext_start, "passed argument is not a path");
    return ext_start;
}