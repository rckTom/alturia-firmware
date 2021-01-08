#include "lua.h"
#include "lauxlib.h"

typedef void (*lua_fcn_handler)(lua_State *L, void *user_data);

struct lua_work_item {
    void *reserved;
    lua_fcn_handler fcn;
    void *data;
};

struct lua_work_item *get_lua_work_item(lua_fcn_handler fcn, size_t user_data_size);
void lua_engine_enque_work(struct lua_work_item *item);
int lua_engine_dofile(const char *filename);
void lua_engine_init();