#include "lua_servolib.h"
#include "lauxlib.h"
#include "servos.h"

/* Set Servo to angle
 * arg1: servo number (int)
 * arg2: servo angle (number)
 */
static int set(lua_State *L)
{
    int servo_num, isnum;
    float angle;
    servo_num = lua_tointegerx(L, -2, &isnum);

    if (!isnum) {
        luaL_error(L, "servo number must be an integer");
    }

    angle = lua_tonumberx(L, -1, &isnum);

    if (!isnum) {
        luaL_error(L, "servo angle mst be a number");
    }

    int iangle = angle;
    servo_set_angle(servo_num, iangle);
    lua_pop(L, 2);

    return 0;
}

static const luaL_Reg libfuncs[] = {
    {"set", set},
    {NULL, NULL},
};

int luaopen_servolib(lua_State *L)
{
    luaL_newlib(L, libfuncs);
    return 1;
}