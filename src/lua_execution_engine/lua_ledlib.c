#include <stdint.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_ledlib.h"
#include "led.h"

static int lua_set_led_color_rgb(lua_State *L)
{
	int isnum; 
	uint32_t color = lua_tointegerx(L, 1, &isnum);
	
	if (isnum) {
		led_set_color_rgb(color);
	}
	lua_pop(L, 1);
	return 0;
}

static int lua_set_led_color_hsv(lua_State *L)
{
	struct color_hsv c;
	int isnum;

	c.h = lua_tonumberx(L, -3, &isnum);
	if (!isnum) {
		goto error;
	}

	c.s = lua_tonumberx(L, -2, &isnum);
	if (!isnum) {
		goto error;
	}

	c.v = lua_tonumberx(L, -1, &isnum);
	if (!isnum) {
		goto error;
	}

	led_set_color_hsv(&c);
	lua_pop(L, 3);
	return 0;

error:
	return luaL_error(L, "argument error");
}

static const luaL_Reg libfuncs[] = {
	{"set_rgb", lua_set_led_color_rgb},
	{"set_hsv", lua_set_led_color_hsv},
	{NULL, NULL}
};

int luaopen_ledlib(lua_State *L)
{
	luaL_newlib(L, libfuncs);
	return 1;
}