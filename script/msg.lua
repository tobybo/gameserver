--定义协议
--PT:上行
--ST:下行

msg = {}
setmetatable(msg,{__index = _G})
_ENV = msg

PT_DIS_CONNECT = 0x0001 --客户端断开连接
PT_PLY_REGIST  = 0x0002 --注册
ST_PLY_REGIST  = 0x0003 --注册
PT_PLY_LOGIN   = 0x0004 --登录
ST_PLY_LOGIN   = 0x0005 --登录
PT_PLY_TIMER   = 0x0006 --定时器测试

PT_MSG_LINE    = 0x1000 --分界线

PT_PLY_CHAT    = 0x1001 --聊天
ST_PLY_CHAT    = 0x1002 --聊天

function handler(self,msg_code,dpid,icq)
	player_t.recored_icq(dpid,icq)
	local fun = msg[msg_code]
	if fun then
		if msg_code > PT_MSG_LINE then
			local player = player_t:get_player_by_dpid(dpid)
			if player then
				fun(player)
			else
				LERR("[MSG] handler, no player, code: %s",msg_code)
			end
		else
			fun(dpid,icq)
		end
	else
		LERR("[MSG] handler, no msg fun, code: %d, dpid: %d, icq: %d",
			msg_code,dpid,icq)
	end
end
