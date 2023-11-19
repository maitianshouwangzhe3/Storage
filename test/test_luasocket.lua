package.cpath = package.cpath .. ";../historage/libhistorage_lua.so"

local storage = require "historage_lua"

local socket = require "socket"

local client = socket.tcp()
client:connect("192.168.138.139", 8000)
--client:settimeout(3)

function manufacturing_data(str, num)
    local index = 1
    local time1 = storage.storage_get_now_time()
    while (index <= num) do
        local data = str .. index .. " " .. index
        client:send(data)
        index = index + 1
    end
    local time2 = storage.storage_get_now_time()
    print("set总共耗时 " .. time2 - time1 .. "ms")
end


print("start...")
--manufacturing_data("set aaaaaaa", 100000)

--os.execute("sleep " .. tonumber(20))

client:send("ping")
res = client:receive("*l")
print(res)

client:close()