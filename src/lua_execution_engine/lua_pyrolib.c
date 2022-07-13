#include "lua_pyrolib.h"
#include "lauxlib.h"
#include "pyros.h"

/* Fire pyro
 * arg1: pyro number (int)
 */
static int fire(lua_State *L)
{
    int isnum;
    int idx = lua_tonumberx(L, -1, &isnum);

    if (!isnum) {
        luaL_error(L, "pyro index must be a number");
    }

    pyros_fire(idx);
    lua_pop(L, 1);

    return 0;
}

static const luaL_Reg libfuncs[] = {
    {"fire", fire},
    {NULL, NULL},
};

int luaopen_pyrolib(lua_State *L)
{
    luaL_newlib(L, libfuncs);
    return 1;
}