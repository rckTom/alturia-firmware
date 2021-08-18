/// Register callbacks which will be executed based on different events
// @module event

#include "lua_eventlib.h"
#include "lua_execution_engine.h"
#include "logging/log.h"
#include "lua.h"
#include "lauxlib.h"
#include "string.h"
#include "events.h"
#include "zephyr.h"
#include "events2.h"


LOG_MODULE_REGISTER(lua_libevent, 3);

static int callback_table_ref;

static void dumpstack (lua_State *L) {
  int top=lua_gettop(L);
  for (int i=1; i <= top; i++) {
    LOG_INF("%d\t%s\t", i, luaL_typename(L,i));
    switch (lua_type(L, i)) {
      case LUA_TNUMBER:
        LOG_INF("%g\n",lua_tonumber(L,i));
        break;
      case LUA_TSTRING:
        LOG_INF("%s\n",lua_tostring(L,i));
        break;
      case LUA_TBOOLEAN:
        LOG_INF("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
        break;
      case LUA_TNIL:
        LOG_INF("%s\n", "nil");
        break;
      default:
        LOG_INF("%p\n",lua_topointer(L,i));
        break;
    }
  }
}


static int execute_callback_impl(lua_State *L, void * user_data)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback_table_ref);
    lua_pushlightuserdata(L, user_data);
    
    if (lua_gettable(L, -2) == LUA_TNIL) {
        lua_settop(L, 0);
        return 0;
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
        lua_call(L, 1, 0);

        //dicard return values
        lua_settop(L, top);
    }

    lua_settop(L, 0);
    return 0;
}

static void push_event_identifier(lua_State *L, const char *name)
{
    char evt_name[32] = {0};
    strcpy(evt_name, name);

    //Make upper case
    for(char *c = evt_name; *c != 0; c++) {
      *c = toupper(*c);
    }

    lua_pushstring(L, evt_name);
}

static void register_event(lua_State *L, const event_t *event)
{
    lua_getglobal(L, "event");

    dumpstack(L);

    if (!lua_istable(L, -1)) {
      lua_pop(L, 1);
      LOG_ERR("not a event type");
      return;
    }

    push_event_identifier(L, event->evt_name);
    lua_pushlightuserdata(L, event);
    lua_settable(L, -3);

    //cleanup stack
    lua_pop(L, 1);
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
      push_event_identifier(L, iterator->evt_name);
      lua_pushlightuserdata(L, iterator);
      lua_settable(L, -3);
    }

    return 1;
}
/***
 * @field EVENT_BOOT
 */