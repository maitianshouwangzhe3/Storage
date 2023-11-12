package.cpath = package.cpath .. ";../historage/libhistorage_lua.so"

local storage = require("historage_lua")
local redis = require "redis"
local maxtime = 0
local fdvec = {}

function max(a, b) 
    if (a > b) then
        return a
    end
    return b
end

function redis_set(client, key, val, num)
    local index = 0
    while (index < num) do
        local tk = key .. index
        local tv = val .. index
        client:set(tk, tv)
        index = index + 1
    end
end

function redis_get(client, key, num)
    local index = 0
    while (index < num) do
        local tk = key .. index
        local time1 = storage.storage_get_now_time()
        local ret = client:get(tk)
        local time2 = storage.storage_get_now_time()
        if ret == "nil" then
            print("get " .. tk .. " fail")
        end

        local time = time2 - time1
        if maxtime < time then
            maxtime = time
        end
        index = index + 1
    end
end

function concurrency_test(ip, port, num, str, index)
    local indexs = 1
    local maxtime = 0
    while (indexs < num) do
        local time1 = storage.storage_get_now_time()
        fdvec[indexs] = redis.connect(ip, port)
        local time2 = storage.storage_get_now_time()
        local time = time2 - time1
        maxtime = max(time, maxtime)
        co = coroutine.create(redis_get)
        coroutine.resume(co, fdvec[indexs], str, index)
        indexs = indexs + 1
    end
    print("连接最大耗时 " .. maxtime .. "ms")
end


print("redis start set...")
local client = redis.connect("192.168.138.139", 6379)
local time1 = storage.storage_get_now_time()
redis_set(client, "aaaaaaa", "aaaaaaa", 100000)
local time2 = storage.storage_get_now_time()
print("set 总共耗时 " .. time2 - time1)

print("redis start get...")
time1 = storage.storage_get_now_time()
concurrency_test("192.168.138.139", 6379, 10000, "aaaaaaa", 10)
time2 = storage.storage_get_now_time()
print("get 总共耗时 " .. time2 - time1 .. " 单次请求最大耗时 " .. maxtime .. " tps = " .. 1000 / maxtime)