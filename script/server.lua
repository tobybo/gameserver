--服务器相关
server = {}
setmetatable(server,{__index = _G})
_ENV = server

module_class(1,"server",resmng.NAME_SPACE_INIT,"namespace")

namespace = namespace or {}

function on_server_start()
	ask_namespace_from_db()
end

function ask_namespace_from_db()
	LLOG("[GAME_START] ask_namespace_from_db, begin.")
	local ret,info = mongo:find(1,"namespace",{_id = 0})
	LLOG("[GAME_START] ask_namespace_from_db, db end, ret: %s, info: %s",tostring(ret),resmng:dump(info))
	if ret and info then
		namespace = wrap(info[1])
		_G.gStep = 2
	else
		info = {_id = 0}
		local ret = mongo:insertOne(1,0,"namespace",info)
		if ret then
			namespace = new(info)
			_G.gStep = 2
		else
			LERR("[GAME_START] ask_namespace_from_db, insert err")
		end
	end
end
