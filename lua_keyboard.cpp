#include "lua_keyboard.h"
#include "lua.hpp"
#include "lua_helpers.h"
#include "keyboard.h"

static int is_held(lua_State* L)
{
    LuaValue li = lua_get_integer(L, 1);

    if (!li.valid)
        Error("ERROR in keyboard.is_held: Expected integer (Key enum) in argument 1.");

    lua_pushboolean(L, key_is_held((Key)li.int_val));
    return 1;
}


static int was_pressed(lua_State* L){
    return 1;

}


static int was_released(lua_State* L){
    return 1;

}

static const struct LuaCFunction funcs [] = {
    {"keyboard_is_held", is_held},
    {"keyboard_was_pressed", was_pressed},
    {"keyboard_was_released", was_released},
    {NULL, NULL}
};

void lua_keyboard_init(lua_State* L)
{
    register_lua_functions(L, funcs);
}