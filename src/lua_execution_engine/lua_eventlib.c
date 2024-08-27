/// Register callbacks which will be executed based on different events
// @module event

#include "lua_eventlib.h"
#include "lua_execution_engine.h"
#include <zephyr/logging/log.h>
#include "lua.h"
#include "lauxlib.h"
#include "string.h"
#include <zephyr/kernel.h>
#include "events2.h"
#include <ctype.h>


LOG_MODULE_REGISTER(lua_libevent, 3);

static int callback_table_ref;

static void execute_callback_impl(lua_State *L, void * user_data)
{
    struct event2 *evt = user_data;
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback_table_ref);
    lua_pushlightuserdata(L, user_data);
    
    if (lua_gettable(L, -2) == LUA_TNIL) {
        lua_settop(L, 0);
        return;
    }

    int callback_count = lua_rawlen(L, -1);
    int top = lua_gettop(L);

    for(int i = 1; i <= callback_count; i++) {
        lua_pushinteger(L, i);
        lua_gettable(L, -2);

        /* Continue if value is not a function */
        if(!lua_isfunction(L, -1)) {
            lua_settop(L, -2);
            continue;
        }

        /* call callback */
        lua_pushlightuserdata(L, user_data);
        if(lua_pcall(L, 1, 0, 0) != 0) {
            luaL_where(L, 1);
            LOG_ERR("Error runnig lua callback for event %s: %s: %s",
                    evt->evt_name,
                    lua_tostring(L, -1),
                    lua_tostring(L, -2));
        }

        //dicard return values
        lua_settop(L, top);
    }

    lua_settop(L, 0);
}

static void execute_callback(struct event2 *evt)
{
    struct lua_work_item *item;
    item = get_lua_work_item(execute_callback_impl, 0);
    item->data = evt;
    lua_engine_enque_work(item);
}

/***
Register a function to execute when the specific event is triggered. 
Multiple callbacks per event can be registred by calling this function multiple times 
@function register_callback
@param event Event to attach callback to
@param callback Callback function which will be executed when event is triggerd
*/
static int register_callback(lua_State *L)
{
    //arguments: event, callback
  
    if(!lua_islightuserdata(L, 1)){
        luaL_error(L, "argument is not a known event");
    }

    event_t *event = (event_t*) lua_touserdata(L, 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback_table_ref); //get callback table from stack
    lua_pushlightuserdata(L, event);
    lua_gettable(L, -2);

    if (lua_isnil(L, -1)) {
        LOG_INF("new table for event");
        lua_pop(L, 1); // pop nil value
        event2_register_callback(event, execute_callback);
        lua_pushlightuserdata(L, event);
        lua_newtable(L); //create new table
        lua_settable(L, 3);
        lua_pushlightuserdata(L, event);
        lua_gettable(L, 3); //get newly created table
    }

    int idx = luaL_len(L, -1) + 1;
    LOG_INF("callback idx %d", idx);
    lua_pushinteger(L, idx);// new index in table
    lua_pushvalue(L, 2); // callback function
    lua_settable(L, 4); // set table at index 4
    lua_settop(L, 0);
    return 0;
}

static const luaL_Reg libfuncs[] = {
    {"register_callback", register_callback},
    {NULL, NULL}
};

int luaopen_eventlib(lua_State *L)
{
    lua_newtable(L);
    callback_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    luaL_newlib(L, libfuncs);

    //make events known at compile time known to lua
    extern struct event2 _event2_list_start[];
    extern struct event2 _event2_list_end[];

    for(struct event2 *iterator = _event2_list_start; iterator != _event2_list_end; iterator++) {
      lua_push_global_identifier(L, iterator->evt_name);
      lua_pushlightuserdata(L, iterator);
      lua_settable(L, -3);
    }

    return 1;
}
/***
 * @field EVENT_BOOT
 */