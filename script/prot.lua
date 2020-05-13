_ENV = player_t

function sendMsg(self,_dpid,msg_code)
	local dpid = _dpid or (self and self.dpid)
	if not dpid then return end
	local icq = player_t.get_icq(dpid)
	if not icq then return end
	c_lib:sendMsg(msg_code,dpid,icq)
end

function send_regist_ret(self,_dpid,account,name)
	c_lib:flushSendBuff();
	c_lib:writeString(account);
	c_lib:writeString(name);
	sendMsg(self,_dpid,msg.ST_PLY_REGIST)
end

function send_chat_msg(self,_dpid,message)
	c_lib:flushSendBuff();
	c_lib:writeString(message);
	sendMsg(self,_dpid,msg.ST_PLY_CHAT)
end
