package.cpath = package.cpath .. ";../historage/libhistorage_lua.so"

local storage = require("historage_lua")

local fd = storage.storage_init("192.168.138.139", 8000)

local str = "quit" 

local res = storage.storage_exec(str, #str, fd)

storage.storage_distory(fd);