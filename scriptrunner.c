#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_cmsgpack.c"

const int MAX_TABLE_NESTING_LEVEL = 100;

static const luaL_Reg whitelisted_libs[] = {
  {"_G", luaopen_base},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_TABLIBNAME, luaopen_table},
  {NULL, NULL}
};

// Makes selected "safe" Lua modules available to the script.
void create_whitelisted_environment (lua_State *L) {
    // See linit.c
    for (const luaL_Reg *lib = whitelisted_libs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);
    }
}

// Throws an error that the Lua script exceeded the allowed instruction count.
void instruction_quota_exceeded (lua_State *L, lua_Debug *ar) {
    fprintf(stderr, "Exceeded instruction quota.\n");
    exit(1);
}

// Uses Lua hooks to limit the number of allowed instructions in the script.
void apply_instruction_quota (lua_State *L, int quota) {
    lua_sethook(L, instruction_quota_exceeded, LUA_MASKCOUNT, quota);
}

// Unsets the instruction limit.
void unapply_instruction_quota (lua_State *L) {
    // Passing 0 as the 3rd argument (the mask) disables the hook.
    lua_sethook(L, instruction_quota_exceeded, 0, 0);
}

// Loads a msgpack object into the Lua environment as a global table variable.
void msgpack_to_lua_table (
    lua_State *L,
    const char *table_name,
    const char *table_msgpack
) {
    // Use lua_cmsgpack to perform the heavy lifting.
    mp_cur c;
    size_t msgpack_length = strlen(table_msgpack);
    mp_cur_init(&c, (const unsigned char *)table_msgpack, msgpack_length);
    mp_decode_to_lua_type(L, &c);
    lua_setglobal(L, table_name);
}

// Encodes the Lua object on the top of the stack to msgpack format.
const char *lua_type_to_msgpack (lua_State *L) {
    mp_buf *buf = mp_buf_new(L);
    mp_encode_lua_type(L, buf, MAX_TABLE_NESTING_LEVEL);

    // Convert buffer to null-terminated string.
    char *msgpack = (char *)malloc(sizeof(char) * (buf->len + 1));
    memcpy(msgpack, buf->b, buf->len);
    msgpack[buf->len] = '\0';
    return msgpack;
}

// Runs a Lua script in a sandboxed environment with some global tables.
const char *run_lua_script (
    const char *script,
    const char *table_name,
    const char *table_msgpack,
    int instruction_quota
) {
    lua_State *L = luaL_newstate();
    create_whitelisted_environment(L);
    int loadStatus = luaL_loadstring(L, script);

    if (loadStatus != LUA_OK) {
        fprintf(stderr, "Failed to load script: %s\n", lua_tostring(L, -1));
        exit(1);
    }

    // Load the global table into the environment.
    msgpack_to_lua_table(L, table_name, table_msgpack);

    // Call the script.
    apply_instruction_quota(L, instruction_quota);
    int callStatus = lua_pcall(L, 0, LUA_MULTRET, 0);
    unapply_instruction_quota(L);

    if (callStatus != LUA_OK) {
        fprintf(stderr, "Failed to run script: %s\n", lua_tostring(L, -1));
        exit(1);
    }

    const char *output_msgpack = lua_type_to_msgpack(L);
    lua_pop(L, 1);
    lua_close(L);
    return output_msgpack;
}
