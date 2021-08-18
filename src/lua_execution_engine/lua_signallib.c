#include "lua_signallib.h"
#include "logging/log.h"
#include "signals.h"
#include "generic.h"
#include "lualib.h"
#include "lauxlib.h"

LOG_MODULE_REGISTER(lua_libsignal, CONFIG_LOG_DEFAULT_LEVEL);

static int get(lua_State *L)
{
    int n = lua_gettop(L);

    if (n != 1) {
        luaL_error(L, "uncorrect number of arguments");
    }

    if (lua_isstring(L, -1)) {
        luaL_error(L, "argument must be a string");
    }

    const char* sig_name = lua_tostring(L, -1);
    struct generic_ptr signal = signal_get(sig_name);
    
    if (signal.type == type_matrix) {
        int r, c;

        r = signal.value_ptr.matrix->numRows;
        c = signal.value_ptr.matrix->numCols;

        lua_createtable(L, 0, signal.value_ptr.matrix->numRows);
        for (int i = 0; i < r; i++) {
            lua_pushinteger(L, i);
            lua_createtable(L, 0, c);
            for( int j = 0; j < c; j++) {
                lua_pushinteger(L, j);
                lua_pushnumber(L, (lua_Number) mat_get((*signal.value_ptr.matrix), i, j));
                lua_settable(L, -3);
            }
            lua_settable(L, -3);
        }

        return 1;
    } else if(signal.type == type_string) {
        lua_pushstring(L, signal.value_ptr.string);
        return 1;
    } else {
        lua_Number num;
        switch (signal.type) {
            case type_float32:
                num = (lua_Number) *signal.value_ptr.float32;
                break;
            case type_int8:
                num = (lua_Number) *signal.value_ptr.int8;
                break;
            case type_int16:
                num = (lua_Number) *signal.value_ptr.int16;
                break;
            case type_int32:
                num = (lua_Number) *signal.value_ptr.int32;
                break;
            case type_int64:
                num = (lua_Number) *signal.value_ptr.int64;
                break;
            case type_uint8:
                num = (lua_Number) *signal.value_ptr.uint8;
                break;
            case type_uint16:
                num = (lua_Number) *signal.value_ptr.uint16;
                break;
            case type_uint32:
                num = (lua_Number) *signal.value_ptr.uint32;
                break;
            case type_uint64:
                num = (lua_Number) *signal.value_ptr.uint64;
                break;
            default:
                num = 0;   
        }
        lua_pushnumber(L, num);
        return 1;
    }

    return 0;
}

static const luaL_Reg libfuncs[] = {
    {"get", get},
    {NULL, NULL}
};

int luaopen_signallib(lua_State *state)
{
    luaL_newlib(state, libfuncs);
    return 1;
}