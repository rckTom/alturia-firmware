#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_ledlib.h"
#include "lua_timerlib.h"
#include "lua_servolib.h"
#include "lua_signallib.h"
#include "lua_eventlib.h"
#include "lua_pyrolib.h"
#include "stdio.h"
#include <zephyr.h>
#include <linker/linker-defs.h>
#include <fs/fs.h>
#include <logging/log.h>
#include "lua_execution_engine.h"
#include <ctype.h>

LOG_MODULE_REGISTER(lua, CONFIG_LOG_DEFAULT_LEVEL);
K_FIFO_DEFINE(lua_work_fifo);

/* put dyanmic lua memory into ccm */
static char __ccm_noinit_section luamem[0xFFFF];
static char lua_read_buf[32];
static struct sys_heap lua_heap;
static lua_State *state;

static const luaL_Reg lua_alturia_libs[] = {
	{"timer", luaopen_timerlib},
	{"led", luaopen_ledlib},
	{"servo", luaopen_servolib},
	{"signal", luaopen_signallib},
	{"event", luaopen_eventlib},
	{"pyro", luaopen_pyrolib},
	{NULL, NULL}
};

void lua_push_global_identifier(lua_State *L, const char *id)
{
    char name[32] = {0};
    strcpy(name, id);

    //Make upper case
    for(char *c = name; *c != 0; c++) {
      *c = toupper(*c);
    }

    lua_pushstring(L, name);
}

static int lua_print(lua_State* L) {
    int nargs = lua_gettop(L);

    for (int i=1; i <= nargs; i++) {
        if (lua_isstring(L, i)) {
            /* Pop the next arg using lua_tostring(L, i) and do your print */
		const char *str = lua_tostring(L,i);
		LOG_INF("%s", str);
        }
        else {
        /* Do something with non-strings if you like */
        }
    }

    return 0;
}

static const struct luaL_Reg printlib [] = {
  {"print", lua_print},
  {NULL, NULL} /* end of array */
};

static int register_global(lua_State *L)
{
  lua_getglobal(L, "_G");
  luaL_setfuncs(L, printlib, 0);  // for Lua versions 5.2 or greater
  lua_pop(L, 1);
  return 0;
}

void *lc_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	(void)ud; (void)osize; /* not used */
	
	if (nsize == 0) {
		sys_heap_free(&lua_heap, ptr);
		return NULL;
	}

	void *p = sys_heap_realloc(&lua_heap, ptr, nsize);
	//sys_heap_dump(&lua_heap);
	return p;
}

const char* lua_reader (lua_State *L, void *data, size_t *size){
	struct fs_file_t *zfp = data;
	size_t rb = fs_read(zfp, lua_read_buf, ARRAY_SIZE(lua_read_buf));
	
	if (rb >= 0) {
		*size = rb;
		return lua_read_buf;
	}

	return NULL;
}


static lua_State* create_state()
{
	sys_heap_init(&lua_heap, luamem, 0xFFFF);
	return lua_newstate(lc_alloc, NULL);
}

static void init_State(lua_State *L)
{
	const luaL_Reg *lib;

	for (lib = lua_alturia_libs; lib->func; lib++) {
		luaL_requiref(L, lib->name, lib->func, 1);
	}
}

static void dofile_impl(lua_State *L, void *data) {
	char *filename = data;
	struct fs_file_t zfp = {0};
	int rc = fs_open(&zfp, filename, FS_O_READ);
	if (rc != 0) {
		LOG_ERR("can not open file %s", filename);
		LOG_ERR("error code %d", rc);
		return;
	}
	lua_load(L, lua_reader, &zfp, filename, NULL);

	if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
		luaL_where(L, 1);
		LOG_ERR("Error runnig lua script %s: %s: %s",
				lua_tostring(L, -1),
				filename,
				lua_tostring(L, -2));
	    lua_settop(L, 0);
	}
}

int lua_engine_dofile(const char* filename)
{
	struct lua_work_item *item;
	size_t fsize = strlen(filename);
	item = get_lua_work_item(dofile_impl, fsize + 1);
	memcpy(item->data, filename, fsize + 1);
	lua_engine_enque_work(item);
	return 0;
}

void lua_engine_init()
{
	state = create_state();
	luaL_openlibs(state);
	init_State(state);
	register_global(state);
}

struct lua_work_item *get_lua_work_item(lua_fcn_handler fcn, size_t user_data_size)
{
	struct lua_work_item *wi = k_malloc(sizeof(struct lua_work_item));
	if (wi == NULL) {
		return NULL;
	}

	if (user_data_size > 0) {
		wi->data = k_malloc(user_data_size);
	}

	if (!wi->data) {
		k_free(wi);
		return NULL;
	}

	wi->fcn = fcn;
	return wi;
}

void lua_engine_enque_work(struct lua_work_item *item) {
	k_fifo_put(&lua_work_fifo, item);
}

static void work_handler()
{
	struct lua_work_item *item;
	while(true) {
		item = k_fifo_get(&lua_work_fifo, K_FOREVER);
		item->fcn(state, item->data);
		k_free(item);
	}
}

K_THREAD_DEFINE(lua_work_handler, 4096, work_handler, NULL, NULL, NULL, CONFIG_LUA_WORKER_PRIO, 0, 0);

