#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_ledlib.h"
#include "lua_timerlib.h"
#include "lua_servolib.h"
#include "stdio.h"
#include <zephyr.h>
#include <linker/linker-defs.h>
#include <fs/fs.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(lua, CONFIG_LOG_DEFAULT_LEVEL);

/* put dyanmic lua memory into ccm */
static char __ccm_noinit_section luamem[0xFFFF];
static char lua_read_buf[32];
struct sys_heap lua_heap;

static const luaL_Reg lua_alturia_libs[] = {
	{"timer", luaopen_timerlib},
	{"led", luaopen_ledlib},
	{"servo", luaopen_servolib},
	{NULL, NULL}
};

void *lc_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	(void)ud; (void)osize; /* not used */
	
	if (nsize == 0) {
		sys_heap_free(&lua_heap, ptr);
		return NULL;
	}

	void *p = sys_heap_realloc(&lua_heap, ptr, nsize);
	sys_heap_dump(&lua_heap);
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

int loadfile(lua_State *L, const char* filename)
{
	struct fs_file_t zfp;
	int rc = fs_open(&zfp, filename, FS_O_READ);
	if (rc != 0) {
		return 0;
	}

	return lua_load(L, lua_reader, &zfp, filename, NULL);
}

lua_State* create_state()
{
	sys_heap_init(&lua_heap, luamem, 0xFFFF);
	return lua_newstate(lc_alloc, NULL);
}

void init_State(lua_State *L)
{
	const luaL_Reg *lib;

	for (lib = lua_alturia_libs; lib->func; lib++) {
		luaL_requiref(L, lib->name, lib->func, 1);
	}
}

