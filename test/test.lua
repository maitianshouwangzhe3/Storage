package.cpath = package.cpath .. ";/home/oyj/storage/test/historage_lua.so"

local storage = require("historage_lua")
local fdvec = {}
local times = 0

function max(a, b) 
    if (a > b) then
        return a
    end
    return b
end

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
        end
        val = val + 1
    end
    if maxtime > times then
        times = maxtime
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
    print("总共耗时 " .. time2 - time1 .. "ms")
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
    local index = 1
    local maxtime = 0
    while (index <= num) do
        local time1 = storage.storage_get_now_time()
        fdvec[index] = storage.storage_init(ip, port)
        local time2 = storage.storage_get_now_time()
        local time = time2 - time1
        maxtime = max(time, maxtime)
        co = coroutine.create(callback)
        coroutine.resume(co, fdvec[index], str, index)
        index = index + 1
        if index % 100 == 0 then
            print("------> " .. index)
        end
    end
    print("连接最大耗时 " .. maxtime .. "ms")
end

function storage_distory_fd()
    for i = 1, #fdvec do
        storage.storage_distory(fdvec[i])
    end
end



fd = storage.storage_init("192.168.138.135", 8000)
manufacturing_data("set aaaaa", fd, 100000)
storage.storage_distory(fd)

storage_concurrency_test("192.168.138.135", 8000, 10000, "get aaaaa", 500)
print("在高并发情况下请求最大耗时 " .. times)
--storage_distory_fd()
