--工具函数
function resmng:build_time_str(_time)
	local time = _time or gTime
	local date_str = os.date("",time)
	return date_str
end
