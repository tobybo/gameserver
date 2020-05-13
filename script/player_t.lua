--玩家
player_t = {}
setmetatable(player_t,{__index = _G})
_ENV = player_t

module_class(0,"player_t",resmng.PLAYER_INIT,"player")

socketMap = socketMap or {
	--[dpid] = {icq,pid}
}

onlinePlys = onlinePlys or {
	--[[
	--[pid] = player
	--
	--]]
}

function incPid()
	local now_pid = server.namespace.max_pid
	server.namespace.max_pid = now_pid + 1
	return server.namespace.max_pid
end

function on_player_regist(dpid,icq,account,pwd,name)
	local filter = {
		["$or"] = {
			{account = account},
			{name = name},
		}
	}
	local ret,info = mongo:find(0,"player",filter)
	if info then
		LERR("[REGIST] on_player_regist, have registed, account: %s, name: %s, info: %s",
			account,name,resmng:dump(info[1]))
		return
	end
	local new_pid = incPid()
	local ply = {_id = new_pid, account = account, name = name, pwd = pwd}
	local ret = mongo:insertOne(0,0,"player",ply)
	if not ret then
		LERR("[REGIST] on_player_regist, insert err, account: %s, name: %s",
			account,name)
		return
	end
	player_t.send_regist_ret(nil,dpid,account,name)
end

function on_player_login(dpid,icq,account,pwd)
	local filter = {
		account = account,
		pwd = pwd,
	}
	local ret,info = mongo:find(0,"player",filter)
	if not info or not info[1] then
		LERR("[LOGIN] on_player_login, no dbinfo, account: %s, pwd: %s",
			account,pwd)
		return
	end
	local node = info[1]
	local player = get_player_by_pid(self,node._id)
	if not player  then
		player = new(node)
	end
	player.dpid = dpid
	socketMap[dpid][2] = player._id
	onlinePlys[player._id] = player
	LLOG("[LOGIN] on_player_login succ, pid: %d, name: %s",player._id,player.name)
end

function get_icq(dpid)
	return socketMap[dpid] and socketMap[dpid][1]
end

function break_player(self)
	onlinePlys[self._id] = nil
end

function get_player_by_pid(self,pid)
	local player = onlinePlys[pid]
	return player
end

function get_player_by_dpid(self,dpid)
	local pid = socketMap[dpid] and socketMap[dpid][2]
	if pid and pid > 0 then return get_player_by_pid(self,pid) end
end

function recored_icq(dpid,icq)
	local node = socketMap[dpid]
	if not node then
		node = {icq}
		socketMap[dpid] = node
	else
		if icq ~= node[1] and node[2] then
			local player = get_player_by_pid(nil,node[2])
			if player then
				player:break_player()
			end
			node[2] = nil
		end
		node[1] = icq
	end
end

function on_msg_disconnect(self,dpid)
	local node = socketMap[dpid]
	if node and node[2] then
		local player = get_player_by_pid(self,node[2])
		if player then
			player:break_player()
			onlinePlys[node[2]] = nil
		end
	end
	socketMap[dpid] = nil
end

function get_socket_plys_count()
	local socket_count = 0
	local plys_count = 0
	for dpid,v in pairs(socketMap) do
		socket_count = socket_count + 1
	end
	for pid,v in pairs(onlinePlys) do
		plys_count = plys_count + 1
	end
	return socket_count,plys_count
end
