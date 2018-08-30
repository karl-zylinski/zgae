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
        Quat quat_val;
        Vec3 vec3_val;
    };
};

struct Vec2i;
LuaValue lua_get_integer(lua_State* L, int arg);
LuaValue lua_get_double(lua_State* L, int arg);
LuaValue lua_get_ptr(lua_State* L, int arg);
LuaValue lua_get_quat(lua_State* L, int arg);
LuaValue lua_get_vec3(lua_State* L, int arg);
LuaValue lua_get_string(lua_State* L, int arg);
void lua_push_vec2i(lua_State* L, const Vec2i& v);