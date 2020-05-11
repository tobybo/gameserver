require("script/json")

c_lib = utils()
log = function(form,...)
	local args = {...}
	if #args > 0 then
		c_lib:log(string.format(form,table.unpack({...})))
	else
		c_lib:log(form)
	end
end

function main_loop(msg_count,dbres_count)
	do_threadpk(msg_count,dbres_count)
end

function do_threadpk(msg_count,dbres_count)
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
				test_write(dpid, icq)
				--req_id,dbnum,opmode,nocallback,collname,sqlstr
				c_lib:flushMongoBuff(1,0,2,1,"test1",encode({["$set"] = {name = "toby1"}}))
				c_lib:writeDocument(encode({name = "toby"}))
				c_lib:runCommandMongo()
			else
				log("lua_error: msg_count > 0 but no msginfo")
			end
		end
	end
	if dbres_count > 0 then
		for i = 1,dbres_count do
			local ret,req_id,opmod,res = c_lib:getDbResInfo();
			log("[LUA_GETDBRS] ret: %d, req_id: %d, opmod: %d, res: %s",
				ret,req_id,opmod,res);
		end
	end
end

function test_write(dpid, icq)
	c_lib:flushSendBuff();
	c_lib:writeString("is toby");
	c_lib:writeInt(26);
	c_lib:writeByte(100);
	c_lib:writeUInt(123456789);
	c_lib:writeUByte(200);
	c_lib:sendMsg(4096,dpid,icq);
end
