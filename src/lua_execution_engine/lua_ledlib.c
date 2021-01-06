#include <stdint.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_ledlib.h"
#include "led.h"

int lua_set_led_color(lua_State *L)
{
	int isnum; 
	uint32_t color = lua_tointegerx(L, 1, &isnum);
	
	if (isnum) {
		led_set_color_rgb(color);
	}
	lua_pop(L, 1);
	return 0;
}

static const luaL_Reg libfuncs[] = {
	{"set_rgb", lua_set_led_color},
	{NULL, NULL}
};

int luaopen_ledlib(lua_State *L)
{
	luaL_newlib(L, libfuncs);
	return 1;
}