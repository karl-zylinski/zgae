#include "path.h"
#include "debug.h"

char* path_ext(char* p)
{
    char* pp = p;
    char* ext_start = NULL;

    while(*pp)
    {
        if((*pp) == '.')
            ext_start = pp + 1;

        ++pp;
    }

    check(ext_start, "passed argument is not a path");
    return ext_start;
}