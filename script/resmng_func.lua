--工具函数
_ENV = resmng

function build_time_str(self,_time)
	local time = _time or gTime
	local date_str = os.date("%Y-%m-%d %H:%M:%S",time)
	return date_str
end

function split_str_pattern(self,src_str,pattern)
	local node = {}
	for v in string.gmatch(src_str,pattern) do
		table.insert(node,v)
	end
	return node
end

function split_str(self,src_str,letter)
	local pattern = string.format("(.[^%s]+)%s",letter,letter)
	return split_str_pattern(self,src_str,pattern)
end

function deep_copy(self,src_tab,dst_tab)
	if type(src_tab) ~= "table" then return src_tab end
	local dst_tab = dst_tab or {}
	for k,v in pairs(src_tab) do
		if k ~= "__index" and k ~= "__newindex" then
			dst_tab[k] = deep_copy(v)
		end
	end
	return dst_tab
end

function dump(self, object)
	if type(object) == 'table' then
		local s = '{ '
		for k,v in pairs(object) do
			if type(k) ~= 'number' then k = "'"..k.."'" end
			s = s .. '['..k..'] = ' .. dump(self,v) .. ','
		end
		return s .. '} '
	elseif type(object) == 'function' then
		return string.dump(object)
	elseif type(object) == 'string' then
		return "'"..tostring(object) .. "'"
	else
		return tostring(object)
	end
end

function undump(self,str)
	local result = nil
	if str then
		local fun = loadstring( "return ".. str  )
		if fun then
			result = fun()
		end
	end
	return result
end
