#pragma once
#include "math.h"

struct lua_State;
struct LuaValue
{
    bool valid;
    union {
        long long int_val;
        const char* str_val;
        double dbl_val;
        void* ptr_val;
        Quaternion quat_val;
        Vector3 vec3_val;
    };
};

LuaValue lua_get_integer(lua_State* L, int arg);
LuaValue lua_get_double(lua_State* L, int arg);
LuaValue lua_get_ptr(lua_State* L, int arg);
LuaValue lua_get_quat(lua_State* L, int arg);
LuaValue lua_get_vec3(lua_State* L, int arg);
LuaValue lua_get_string(lua_State* L, int arg);