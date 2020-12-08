
local VERSION = require('poco.lua.POCO_SDK_VERSION')
local ClientConnection = require('poco.lua.ClientConnection')

local PocoManager = {}
PocoManager.__index = PocoManager

PocoManager.DEBUG = false
PocoManager.VERSION = VERSION

PocoManager.server_sock = nil
PocoManager.all_socks = {}
PocoManager.clients = {}

-- from unit space to screen space
local function unit_to_screen(x, y)
    local w, h = window.get_size()
    return x * w, y * h
end

-- rpc methods registration
local dispatcher = {
    GetSDKVersion = function() return VERSION end,
    Dump = function(onlyVisibleNode)
        if onlyVisibleNode == nil then
            onlyVisibleNode = true
        end

        if poco_helper ~=nil then
            local w, h = window.get_size()
            return poco_helper.dump(onlyVisibleNode, w, h)
        end
        return "No poco_helper extension found!"
    end,
    Click = function(x, y)
        x, y = unit_to_screen(x, y)
        poco_helper.click(x, y)
        return {}
    end,
    LongClick = function(x, y, duration)
        x, y = unit_to_screen(x, y)
        poco_helper.long_click(x, y, duration)
        return {}
    end,
    -- RClick = function(x, y)
    --     local w, h = window.get_size()
    --     poco_helper.right_click(x * w, y * h)
    --     return {}
    -- end,
    -- DoubleClick = function(x, y)
    --     local w, h = window.get_size()
    --     poco_helper.double_click(x * w, y * h)
    --     return {}
    -- end,

    Swipe = function(x1, y1, x2, y2, duration)
        x1, y1 = unit_to_screen(x1, y1)
        x2, y2 = unit_to_screen(x2, y2)
        poco_helper.swipe(x1, y1, x2, y2, duration)
        return {}
    end,

    -- Screenshot = function(width)
    --     width = width or 720
    --     return Screen:getScreen(width)
    -- end,
    GetScreenSize = function()
        local w, h = window.get_size()
        return {width = w, height = h}
    end,
    -- NOTE: We currently have no good way of returning a unique ID from Defold to Poco that
    -- contains the URL but also the sub component (for gui nodes)
    -- SetText = function(_instanceId, val)
    --     print("SetText", _instanceId, val)
    --     local node = Dumper:getCachedNode(_instanceId)
    --     if node ~= nil then
    --         return node:setAttr('text', val)
    --     end
    --     return false
    -- end,
    KeyEvent = function(val)
        poco_helper.keyevent(val)
        return true
    end,
    Execute = function(msg)
        local f = loadstring(msg)
        local res = f()
        if res ~= nil then
            return res
        end
        return ""
    end,
}

local callbacks = {
}


function PocoManager:init_server(port)
    port = port or 15004
    local server_sock, err = socket.tcp()
    assert(server_sock)
    table.insert(self.all_socks, server_sock)
    self.server_sock = server_sock
    server_sock:setoption('reuseaddr', true)
    server_sock:setoption('keepalive', true)
    server_sock:settimeout(0.0)
    server_sock:bind('*', port)
    server_sock:listen(5)
    print(string.format('[poco] server listens on tcp://*:%s', port))
end

-- TODO: perhaps drive this automatically from the extension update loop?
function PocoManager:server_loop()
    for _, c in pairs(self.clients) do
        c:drainOutputBuffer()
    end

    local r, w, e = socket.select(self.all_socks, nil, 0)
    if #r > 0 then
        local removed_socks = {}

        for i, v in ipairs(r) do
            if v == self.server_sock then
                local client_sock, err = self.server_sock:accept()
                print('[poco] new client accepted', client_sock:getpeername(), err)
                table.insert(self.all_socks, client_sock)
                self.clients[client_sock] = ClientConnection:new(client_sock, self.DEBUG)
            else
                local client = self.clients[v]
                local reqs = client:receive()
                if reqs == '' then
                    -- client is gone
                    self.clients[v] = nil
                    table.insert(removed_socks, v)
                elseif reqs ~= nil then
                    for _, req in ipairs(reqs) do
                        self:onRequest(req)
                    end
                end
            end
        end

        for _, s in pairs(removed_socks) do
            for i, v in ipairs(self.all_socks) do
                if v == s then
                    table.remove(self.all_socks, i)
                    break  -- break inner loop only
                end
            end
        end
    end

    for _, c in pairs(self.clients) do
        c:drainOutputBuffer()
    end
end


function PocoManager:setDispatchFn(method, fn)
    dispatcher[method] = fn
end

function PocoManager:setDispatchCallbackFn(method, fn)
    callbacks[method] = fn
end

function PocoManager:onRequest(req)
    local client = req.client
    local method = req.method
    local params = req.params
    local func = dispatcher[method]
    local client_callback = callbacks[method]
    local ret = {
        id = req.id,
        jsonrpc = req.jsonrpc,
        result = nil,
        error = nil,
    }
    if func == nil then
        ret.error = {message = string.format('No such rpc method "%s", reqid: %s, client:%s', method, req.id, req.client:getAddress())}
        client:send(ret)
    else
        xpcall(function()
            local result = func(unpack(params))
            if type(result) == 'function' then
                result(function(cbresult)
                    ret.result = cbresult
                    client:send(ret)
                end)
                return
            else
                if client_callback ~= nil then
                    local newresult = client_callback(result)
                    if newresult == nil then
                        local err = string.format('Client callback for rpc method "%s" returned `nil`', method)
                        print("[poco] Error: ", err)
                    else
                        result = newresult
                    end
                end
                ret.result = result
                client:send(ret)
            end
        end, function(msg)
            ret.error = {message = debug.traceback(msg)}
            client:send(ret)
        end)
    end
end

return PocoManager
