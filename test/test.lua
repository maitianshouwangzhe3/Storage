package.cpath = package.cpath .. ";../historage/libhistorage_lua.so"

local storage = require("historage_lua")
local fdvec = {}
local times_get = 0
local times_query = 0
local zsetkey = {
    key = "aaaaaaa",
    val = 1,
    name = "aaaaaaa",
}

function max(a, b) 
    if (a > b) then
        return a
    end
    return b
end

--get命令测试回调函数
function callback(fd, str, index)
    local val = 1
    local maxtime = 0
    while (val <= index) do
        local data = str .. val .. " " .. val
        local time1 = storage.storage_get_now_time()
        local res = storage.storage_exec(data, #data, fd)
        local time2 = storage.storage_get_now_time()
        if (res ~= "null") then
            maxtime = max(maxtime, time2 -time1)
        else
            print(data .. " fail")
        end
        val = val + 1
    end
    if maxtime > times_get then
        times_get = maxtime
    end
    storage.storage_distory(fd)
end

--query命令测试回调函数
function callback2(fd, index)
    local val = 1
    local maxtime = 0
    while (val <= index) do
        local data = "query " .. zsetkey.key .. " " .. (zsetkey.val + val) .. " " .. zsetkey.name .. val .. " 0 4" 
        local time1 = storage.storage_get_now_time()
        local res = storage.storage_exec(data, #data, fd)
        local time2 = storage.storage_get_now_time()
        if (res ~= "null") then
            maxtime = max(maxtime, time2 -time1)
        else
            print(data .. " fail")
        end
        val = val + 1
    end
    if maxtime > times_query then
        times_query =  maxtime
    end
    storage.storage_distory(fd)
end

--set 数据 str为字符串 fd sock套接字 数据数量  数据会是str + num的递增值
function manufacturing_data(str, fd, num)
    local index = 1
    local time1 = storage.storage_get_now_time()
    while (index <= num) do
        local data = str .. index .. " " .. index
        local res = storage.storage_exec(data, #data, fd)
        if (res ~= "OK") then
            print(data .. " fail")
        end
        index = index + 1
    end
    local time2 = storage.storage_get_now_time()
    print("set总共耗时 " .. time2 - time1 .. "ms")
end

-- 测试get最大耗时 str为字符串 fd sock套接字 num为测试次数
function storage_get_test(str, fd, num)
    local index = 1
    local maxtime = 0
    while (index <= num) do
        local data = str .. index
        local time1 = storage.storage_get_now_time()
        local res = storage.storage_exec(data, #data, fd)
        local time2 = storage.storage_get_now_time()
        local time = time2 - time1
        maxtime = max(time, maxtime)
        if (res == "null") then
            print(data .. " null")
        end
    end
    print("最大耗时 " .. maxtime .. "ms")
end

-- 测试并发性能
function storage_concurrency_test(ip, port, num, str, index)
    local indexs = 1
    local maxtime = 0
    while (indexs <= num) do
        local time1 = storage.storage_get_now_time()
        fdvec[indexs] = storage.storage_init(ip, port)
        local time2 = storage.storage_get_now_time()
        local time = time2 - time1
        maxtime = max(time, maxtime)
        co = coroutine.create(callback)
        coroutine.resume(co, fdvec[indexs], str, index)
        indexs = indexs + 1
    end
    print("连接最大耗时 " .. maxtime .. "ms")
end

function storage_distory_fd()
    for i = 1, #fdvec do
        storage.storage_distory(fdvec[i])
    end
end

function storage_concurrency_test_zset(ip, port, num, str, index)
    local indexs = 1
    local maxtime = 0
    while (indexs <= num) do
        local time1 = storage.storage_get_now_time()
        fdvec[indexs] = storage.storage_init(ip, port)
        local time2 = storage.storage_get_now_time()
        local time = time2 - time1
        maxtime = max(time, maxtime)
        co = coroutine.create(callback2)
        coroutine.resume(co, fdvec[indexs], index)
        indexs = indexs + 1
    end
    print("连接最大耗时 " .. maxtime .. "ms")
end

function manufacturing_data_zset(fd, num)
    local index = 1
    local time1 = storage.storage_get_now_time()
    while (index <= num) do
        local buf = "zset " .. zsetkey.key .. " " .. (zsetkey.val + index) .. " " .. zsetkey.name .. tostring(index)
        local ret = storage.storage_exec(buf, #buf, fd)
        if ret == "null" then
            print(buf .. " fail")
        end
        index = index + 1
    end
    local time2 = storage.storage_get_now_time()
    print("zset总共耗时 " .. time2 - time1 .. "ms")
end

local datasize = 100000
local clientsize = 10000


fd = storage.storage_init("192.168.138.135", 8000)
print("开始准备数据...")
manufacturing_data("set aaaaa", fd, datasize)
manufacturing_data_zset(fd, datasize)
storage.storage_distory(fd)

print("开始测试get命令...")
local time1 = storage.storage_get_now_time()
storage_concurrency_test("192.168.138.135", 8000, clientsize, "get aaaaa", 5)
local time2 = storage.storage_get_now_time()
--tps直接用单词请求最大耗时算
print("在客户端数量为" .. clientsize .. "情况下get请求最大耗时 " .. times_get .. "ms " .. "数据量 " .. datasize .. " TPS=" .. (1000 / times_get))

print("开始测试query命令...")
time1 = storage.storage_get_now_time()
storage_concurrency_test_zset("192.168.138.135", 8000, clientsize, zsetkey, 10)
time2 = storage.storage_get_now_time()
print("在客户端数量为" .. clientsize .. "情况下query请求最大耗时 " .. times_query .. "ms " .. "数据量 " .. datasize .. " TPS=" ..  (1000 / times_query))

