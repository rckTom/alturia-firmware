#include "lua.h"
#include "lauxlib.h"

lua_State *create_state(void);
void init_State(lua_State *L);
int loadfile(lua_State *L, const char *filename);