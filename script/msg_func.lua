--上行方法
_ENV = msg

msg[PT_DIS_CONNECT] = function(dpid,icq)
	LLOG("[MSG] disconnect msg, dpid: %d, icq: %d",dpid,icq)
	player_t:on_msg_disconnect(dpid)
end

msg[PT_PLY_REGIST] = function(dpid,icq)
	local account = c_lib:readString()
	local pwd = c_lib:readString()
	local name = c_lib:readString()
	player_t.on_player_regist(dpid,icq,account,pwd,name)
end

msg[PT_PLY_LOGIN] = function(dpid,icq)
	local account = c_lib:readString()
	local pwd = c_lib:readString()
	player_t.on_player_login(dpid,icq,account,pwd)
end

msg[PT_PLY_TIMER] = function(dpid,icq)
	local info = c_lib:readString()
	timer:addTimer(1000,1,1,2,"3","4");
	timer:addTimer(1000,2,1,2,"3","4");
	timer:addTimer(2000,3,1,2,"3","4");
	timer:addTimer(3000,4,1,2,"3","4");
	timer:addTimer(4000,5,1,2,"3","4");
end

msg[PT_PLY_CHAT] = function(player)
	local message = c_lib:readString()
	c_lib:flushSendBuff();
	c_lib:writeString(message);
	for pid,ply in pairs(player_t.onlinePlys) do
		if pid ~= ply._id then
			ply:sendMsg(nil,msg.ST_PLY_CHAT)
		end
	end
end
