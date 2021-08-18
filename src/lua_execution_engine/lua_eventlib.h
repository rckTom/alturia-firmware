#ifndef ALTURIA__LUA_EVENTLIB__H
#define ALTURIA__LUA_EVENTLIB__H

#include "lua.h"

/*** @module event
 * @field EVENT_APOGEE
 * @field EVENT_TOUCHDOWN
 * @field EVENT_IGNITION
 * @field EVENT_BURNOUT
 */

int luaopen_eventlib(lua_State *L);

#endif