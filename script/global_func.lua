--一些基础的全局方法 通常不会被reload
--初始化c++层函数

log = function(form,...)
	local date_str = resmng:build_time_str()
	local args = {...}
	if #args > 0 then
		c_lib:log(string.format("[%s][%s]"..form,date_str,gFrame,table.unpack({...})))
	else
		c_lib:log(string.format("[%s][%s]"..form,gFrame,date_str))
	end
end

LLOG = function(_form,...)
	local form = string.format("[LOG]%s",_form)
	log(form,...)
end

LWARN = function(_form,...)
	local form = string.format("[WARN]%s",_form)
	log(form,...)
end

LERR = function(_form,...)
	local form = string.format("[ERROR]%s",_form)
	log(form,...)
end
