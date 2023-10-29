#ifndef __HISTORAGE_LUA_H__
#define __HISTORAGE_LUA_H__


#include <lua.h>
#include <lauxlib.h>

int storage_init_lua(lua_State* L);
void storage_distory_lua(lua_State* L);
void storage_exec_lua(lua_State* L);


#endif