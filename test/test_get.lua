package.cpath = package.cpath .. ";/home/oyj/storage/test/historage_lua.so"

local storage = require("historage_lua")

local fd = storage.storage_init("192.168.138.135", 8000)

local str = "get aaaaa1200" 
local time1 = storage.storage_get_now_time()
local res = storage.storage_exec(str, #str, fd)
local time2 = storage.storage_get_now_time()
print("storage return " .. res)
print("耗时 " .. time2 - time1 .. "ms")
storage.storage_distory(fd);