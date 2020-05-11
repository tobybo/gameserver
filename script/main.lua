require("script/json")
require("script/resmng")

gTime = gTime or 0

c_lib = utils()
log = function(form,...)
	local date_str = resmng:build_time_str()
	local args = {...}
	if #args > 0 then
		c_lib:log(string.format("[%s]"..form,date_str,table.unpack({...})))
	else
		c_lib:log(string.format("[%s]"..form,date_str))
	end
end

LLOG = function(_form,...)
	local form = string.format("[LOG] %s",_form)
	log(form,...)
end

LWARN = function(_form,...)
	local form = string.format("[WARN] %s",_form)
	log(form,...)
end

LERR = function(_form,...)
	local form = string.format("[ERROR] %s",_form)
	log(form,...)
end

function main_loop(gtime,msg_count,dbres_count)
	gTime = gtime
	do_threadpk(msg_count,dbres_count)
end

function do_threadpk(msg_count,dbres_count)
	if msg_count > 0 then
		for i = 1,msg_count do
			local msg_code,dpid,icq = c_lib:getMsgInfo()
			if msg_code > 0 then
				msg:handler(msg_code,dpid,icq)
				--[[log("[   get one msg, code: %d, dpid: %s, icq: %s ]",msg_code,tostring(dpid),tostring(icq))]]
				--local msg = c_lib:readString()
				--log("[   msg info: %s ]",msg)
				--msg = c_lib:readString()
				--log("[   msg info: %s ]",msg)
				--msg = c_lib:readString()
				--log("[   msg info: %s ]",msg)
				--test_write(dpid, icq)
				----req_id,dbnum,opmode,nocallback,collname,sqlstr
				--c_lib:flushMongoBuff(1,0,2,1,"test1",encode({["$set"] = {name = "toby1"}}))
				--c_lib:writeDocument(encode({name = "toby"}))
				--[[c_lib:runCommandMongo()]]
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
