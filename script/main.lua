c_lib = c_lib or utils()

--可以reload的模块
module = function(path)
	if package.loaded[path] then
		package.loaded[path] = nil
	end
	require(path)
	c_lib:log(string.format("[GLOBAL] module path: %s",path))
end

module("script/json")
module("script/resmng")
module("script/resmng_func")
module("script/define")
module("script/global_func")
module("script/msg")
module("script/msg_func")
module("script/timer")
module("script/mongo")
module("script/pending_save")
module("script/server")
module("script/player_t")
module("script/prot")

gTime = gTime or 0
gStep = gStep or 0 --服务器状态 0 未初始化 1 就绪
gFrame = gFrame or 0

gMongoCo = gMongoCo or {}
gCoPool  = gCoPool  or {}
gActions = gActions or {}

function main_loop(gtime,gframe,msg_count,dbres_count,timer_count)
	gTime = gtime
	gFrame = gframe
	if gFrame % 20 == 0 then
		local s_count,p_count = player_t:get_socket_plys_count()
		LLOG("[SERVER] main_loop, step: %d, msg_count: %d, dbres_count: %d, socket_count: %d, online_count: %d",
			gStep,msg_count,dbres_count,s_count,p_count)
	end
	if gStep == 0 then
		gStep = 1
		action(on_server_init)
	end
	if gStep == 2 then
		if msg_count > 0 then
			while true do
				local co = get_co_from_pool("pk")
				local ret,flag = coroutine.resume(co)
				if ret then
					if flag == "ok" then break end --说明消息处理完了
				else
					LERR("[MAIN_LOOP] resume pk co err")
				end
			end
		end
	end
	if dbres_count > 0 then
		while true do
			local co = get_co_from_pool("db")
			local ret,flag = coroutine.resume(co)
			if ret then
				if flag == "ok" then break end --说明消息处理完了
			else
				LERR("[MAIN_LOOP] resume db co err")
			end
		end
	end
	if timer_count > 0 then
		while true do
			local co = get_co_from_pool("timer")
			local ret,flag = coroutine.resume(co)
			if ret then
				if flag == "ok" then break end --说明消息处理完了
			else
				LERR("[MAIN_LOOP] resume timer co err")
			end
		end
	end
	if #gActions > 0 then
		while true do
			local co = get_co_from_pool("ac")
			local ret,flag = coroutine.resume(co)
			if ret then
				if flag == "ok" then break end --说明消息处理完了
			else
				LERR("[MAIN_LOOP] resume ac co err, %s",flag)
			end
		end
	end
	action(server.do_pending_save)
end

function do_threaddb()
	while true do
		local dbres_count = c_lib:getDbResCount()
		if dbres_count > 0 then
			for i = 1,dbres_count do
				if c_lib:getDbResCount() > 0 then
					local ret,req_id,opmod,res = c_lib:getDbResInfo();
					LLOG("[LUA_GETDBRS] ret: %d, req_id: %d, opmod: %d, res: %s",
					ret,req_id,opmod,res);
					mongo:popDbCo(ret,req_id,opmod,res)
				else
					break
				end
			end
		end
		put_co_into_pool("db")
	end
end

function do_threadac()
	while true do
		local old_count = #gActions
		while old_count > 0 do
			old_count = old_count - 1
			if #gActions > 0 then
				local node = table.remove(gActions,1) --先入先出
				node[1](table.unpack(node[2]))
			end
		end
		put_co_into_pool("action")
	end
end

function do_threadpk()
	while true do
		local msg_count = c_lib:getMsgCount()
		if msg_count > 0 then
			for i = 1,msg_count do --限制一帧处理的消息量
				if c_lib:getMsgCount() > 0 then --避免执行处理函数时挂起而导致消息数不可靠的问题
					local msg_code,dpid,icq = c_lib:getMsgInfo()
					if msg_code > 0 then
						msg:handler(msg_code,dpid,icq)
					else
						log("lua_error: msg_count > 0 but no msginfo")
					end
				else
					break
				end
			end
		end
		put_co_into_pool("pk")
	end
end

function do_threadtimer()
	while true do
		local timer_count = c_lib:getTimerCount()
		if timer_count > 0 then
			for i = 1,timer_count do --限制一帧处理的消息量
				if c_lib:getTimerCount() > 0 then --避免执行处理函数时挂起而导致消息数不可靠的问题
					local eventId,p1,p2,p3,p4 = c_lib:getTimerInfo()
					if eventId > 0 then
						timer:handlerTimer(eventId,p1,p2,p3,p4)
					else
						log("lua_error: timer_count > 0 but no timerinfo")
					end
				else
					break
				end
			end
		end
		put_co_into_pool("timer")
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

function on_server_init()
	server:on_server_start()
end

function STACK(err)
	local stacks = debug.traceback( err, 2  )
	for s in string.gmatch( stacks, "[^%c]+"  ) do
		LWARN( string.format( "[LUA] %s", s  )  )
	end
	LWARN( string.format( "[LuaError] catch,  %s", string.gsub( stacks, "\n\t?", "#012"  )  )  )
end

function pcall_threadpk()
	xpcall(do_threadpk,STACK)
end

function pcall_threaddb()
	xpcall(do_threaddb,STACK)
end

function pcall_threadac()
	xpcall(do_threadac,STACK)
end

function pcall_threadtimer()
	xpcall(do_threadtimer,STACK)
end

function create_co(what)
	if what == "pk" then
		return coroutine.create(pcall_threadpk)
	elseif what == "db" then
		return coroutine.create(pcall_threaddb)
	elseif what == "ac" then
		return coroutine.create(pcall_threadac)
	elseif what == "timer" then
		return coroutine.create(pcall_threadtimer)
	end
end

function get_co_from_pool(what)
	if gCoPool[what] and #gCoPool[what] > 0 then
		local co = table.remove(gCoPool[what])
		return co
	else
		local co = create_co(what)
		return co
	end
end

function put_co_into_pool(what)
	local co = coroutine.running()
	gCoPool[what] = gCoPool[what] or {}
	if #gCoPool[what] < 10 then table.insert(gCoPool[what],co) end
	coroutine.yield("ok")
end

function action(func,...)
	local args = {...}
	local node = {func,args}
	table.insert(gActions,node)
end
