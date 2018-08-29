#include "lua_helpers.h"
#include "lua.hpp"

LuaValue lua_get_integer(lua_State* L, int arg)
{
    if (!lua_isnumber(L, arg))
    {
        return {false};
    }

    return {true, lua_tointeger(L, arg)};
}


LuaValue lua_get_double(lua_State* L, int arg)
{
    if (!lua_isnumber(L, arg))
    {
        return {false};
    }

    LuaValue lv = {};
    lv.valid = true;
    lv.dbl_val = lua_tonumber(L, arg);
    return lv;
}

LuaValue lua_get_ptr(lua_State* L, int arg)
{
    if (!lua_isuserdata(L, arg))
    {
        return {false};
    }

    LuaValue lv = {};
    lv.valid = true;
    lv.ptr_val = lua_touserdata(L, arg);
    return lv;
}

LuaValue lua_get_quat(lua_State* L, int arg)
{
    lua_getfield(L, arg, "w");
    lua_getfield(L, arg, "z");
    lua_getfield(L, arg, "y");
    lua_getfield(L, arg, "x");
    LuaValue lx = lua_get_double(L, -1);
    LuaValue ly = lua_get_double(L, -2);
    LuaValue lz = lua_get_double(L, -3);
    LuaValue lw = lua_get_double(L, -4);
    lua_pop(L, 4);

    if (!lx.valid || !ly.valid || !lz.valid || !lw.valid)
        return {false};

    Quat q = {(float)lx.dbl_val, (float)ly.dbl_val, (float)lz.dbl_val, (float)lw.dbl_val };
    LuaValue lv = {};
    lv.valid = true;
    lv.quat_val = q;
    return lv;
}

LuaValue lua_get_vec3(lua_State* L, int arg)
{
    lua_getfield(L, arg, "z");
    lua_getfield(L, arg, "y");
    lua_getfield(L, arg, "x");
    LuaValue lx = lua_get_double(L, -1);
    LuaValue ly = lua_get_double(L, -2);
    LuaValue lz = lua_get_double(L, -3);
    lua_pop(L, 3);

    if (!lx.valid || !ly.valid || !lz.valid)
        return {false};

    Vec3 v = {(float)lx.dbl_val, (float)ly.dbl_val, (float)lz.dbl_val };
    LuaValue lv = {};
    lv.valid = true;
    lv.vec3_val = v;
    return lv;
}

LuaValue lua_get_string(lua_State* L, int arg)
{
    if (!lua_isstring(L, arg))
    {
        return {false};
    }

    const char* str = lua_tostring(L, arg);
    LuaValue lv = {true};
    lv.str_val = str;
    return lv;
}