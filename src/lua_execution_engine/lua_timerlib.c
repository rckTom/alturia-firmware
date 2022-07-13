#include "lua.h"
#include <zephyr.h>
#include "lauxlib.h"
#include "lualib.h"
#include "eventtimer.h"
#include <logging/log.h>
#include "lua_execution_engine.h"

LOG_MODULE_REGISTER(lua_libtimer, CONFIG_LOG_DEFAULT_LEVEL);

/* Start eventtimer
 * arg1: timer number (int)
 * arg2: period (int)
 * arg3: cyclic (bool)
 */

static int callback_refs[CONFIG_EVENT_TIMERS_NUMBER];

static int start(lua_State *L)
{
    int timer_num, period, isnum;
    timer_num = lua_tointegerx(L, -3, &isnum);

    if (!isnum) {
        return luaL_error(L, "timer number must be a integer");
    }

    period = lua_tointegerx(L, -2, &isnum);

    if (!isnum) {
        return luaL_error(L, "period must be an integer specifing the time in milliseconds");
    }

    bool cyclic = lua_toboolean(L, -1);

    lua_pop(L, 3);
    int rc = event_timer_start(timer_num, period, cyclic);
    
    if (rc != 0) {
        return luaL_error(L, "internal error");
    }

    return 0;
}

/* Stop event timer
 * arg1: timer number (int)
 */
static int stop(lua_State *L)
{
    int timer_num, isnum;
    timer_num = lua_tointegerx(L, -1, &isnum);
    lua_pop(L, 1);
    if (!isnum) {
        luaL_error(L, "timer number must be an integer");
    }

    int rc = event_timer_stop(timer_num);

    if (rc != 0) {
        luaL_error(L, "internal error");
    }

    event_timer_stop(timer_num);

    return 0;
}

/* Register callback
 * arg1: callback function
 */
static int register_callback(lua_State *L)
{
    int isnum, timer_num;
    timer_num = lua_tointegerx(L, -2, &isnum);
    if (!isnum) {
        return luaL_error(L, "timer number must be an integer");
        lua_pop(L, 2);
    }

    //store function in registry. compose string 
    callback_refs[timer_num] = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1);
    return 0;
}

static int timer_expire(lua_State *L)
{
    int timer_num = lua_tointeger(L, -1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback_refs[timer_num]);
    lua_pcall(L, 0, 0, 0);
    lua_pop(L, 1);
    return 0;
}

static void execute_callback_impl(lua_State *L, void *user_data) {
    int timer_number = *(int *)user_data;
    k_free(user_data);
    lua_pushcfunction(L, timer_expire);
    lua_pushinteger(L, timer_number);
    if (lua_pcall(L, 1, 0, 0)) {
        LOG_ERR("%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static void execute_callback(int timer_number)
{
    struct lua_work_item *item;
    item = get_lua_work_item(execute_callback_impl, sizeof(int));
    *(int*)item->data = timer_number;
    lua_engine_enque_work(item);
}

static const luaL_Reg libfuncs[] = {
    {"start", start},
    {"stop", stop},
    {"register_callback", register_callback},
    {NULL, NULL}
};

int luaopen_timerlib(lua_State *L)
{
    luaL_newlib(L, libfuncs);
    event_timer_set_callback(execute_callback);
    return 1;
}
