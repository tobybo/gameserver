--定义协议
--PT:上行
--ST:下行

msg = {}

msg.PT_DIS_CONNECT = 0x0001 --客户端断开连接

msg.PT_PLY_REGIST  = 0x1001 --注册
msg.ST_PLY_REGIST  = 0x1002 --注册
msg.PT_PLY_LOGIN   = 0x1003 --登录
msg.ST_PLY_LOGIN   = 0x1004 --登录
msg.PT_PLY_CHAT    = 0x1005 --聊天
msg.ST_PLY_CHAT    = 0x1006 --聊天

require("script/msg_func")

function msg:handler(msg_code,dpid,icq)
	local fun = msg[msg_code]
	if fun then
		fun(dpid,icq)
	else
		LERR("[MSG] handler, no msg fun, code: %d, dpid: %d, icq: %d",
			msg_code,dpid,icq)
	end
end
