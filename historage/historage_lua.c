
//include "historage.h"
#include <lua.h>
#include <lauxlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <time.h>  
#include <sys/time.h>  

int storage_init(const char* ip, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(port);
    addr.sin_addr.s_addr = inet_addr(ip);  
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if(rv){
        return -1;
    }

    return fd;
}

void storage_distory(int sock)
{
    close(sock);
}

void storage_exec(const char* buf, int buflen, const char* com, int comlen ,int fd){
    if(fd < 0 || comlen <= 0){
        return;
    }

    int ret = send(fd, com, comlen, 0);
    if(ret <= 0){
        return;
    }

    ret = recv(fd, (void*)buf, buflen, 0);
    if(ret <= 0){
        strcpy((char*)buf, "error");
    }
}

long long storage_get_now_time() {
    struct timeval tv;  
    gettimeofday(&tv, NULL);  
    long long milliseconds = tv.tv_sec * 1000LL + tv.tv_usec / 1000; 
    return milliseconds;
}


int storage_init_lua(lua_State* L) {
    const char* ip = luaL_checkstring(L, 1);
    int port = luaL_checkinteger(L, 2);

    int fd = storage_init(ip, port);

    lua_pushinteger(L, fd);
    return 1;
}

void storage_distory_lua(lua_State* L) {
    int fd = luaL_checkinteger(L, 1);

    storage_distory(fd);
}

int storage_exec_lua(lua_State* L) {
    const char* com = luaL_checkstring(L, 1);
    int comlen = luaL_checkinteger(L, 2);
    int fd = luaL_checkinteger(L, 3);

    char buf[256] = {0};
    storage_exec(buf, sizeof(buf), com, comlen, fd);

    lua_pushlstring(L, buf, strlen(buf));
    return 1;
}

int storage_get_now_time_lua(lua_State* L) {
    long long time = storage_get_now_time();

    lua_pushinteger(L, time);
    return 1;
}

static const struct luaL_Reg example_funcs[] = {  
        {"storage_init", storage_init_lua},  // 将你的函数名替换为实际的函数名  
        {"storage_distory", storage_distory_lua}, 
        {"storage_exec", storage_exec_lua}, 
        {"storage_get_now_time", storage_get_now_time_lua}, 
        {NULL, NULL}  
    };


LUALIB_API int luaopen_historage_lua(lua_State *L) {  
      
    luaL_newlib(L, example_funcs);  
    return 1;  
}