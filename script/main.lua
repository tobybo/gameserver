--require "gamesvr.lua"


c_lib = utils()
log = function(form,...)
	local args = {...}
	if #args > 0 then
		c_lib:log(string.format(form,table.unpack({...})))
	else
		c_lib:log(form)
	end
end

function main_loop(msg_count)
	do_threadpk(msg_count)
end

function do_threadpk(msg_count)
	if msg_count > 0 then
		for i = 1,msg_count do
			local msg_code,dpid,icq = c_lib:getMsgInfo()
			if msg_code > 0 then
				log("[   get one msg, code: %d, dpid: %s, icq: %s ]",msg_code,tostring(dpid),tostring(icq))
				local msg = c_lib:readString()
				log("[   msg info: %s ]",msg)
				msg = c_lib:readString()
				log("[   msg info: %s ]",msg)
				msg = c_lib:readString()
				log("[   msg info: %s ]",msg)
			else
				log("lua_error: msg_count > 0 but no msginfo")
			end
		end
	end
end

--loop()
