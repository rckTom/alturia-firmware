#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "daq.h"
#include "util.h"

int lua_get_sensor_data(lua_State *L)
{
    lua_createtable(L, 0, 0);
    lua_pushnumber(L, sensor_value_to_float(&press_sample));
    lua_setfield(L, -2, "pressure");
    return 1;
}

static const luaL_Reg libfuncs[] = {
    {"get_sample", lua_get_sensor_data},
    {NULL, NULL}
};

int luaopen_alturialib(lua_State *state)
{
    luaL_newlib(state, libfuncs);
    return 1;
}