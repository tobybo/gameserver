--末帧入库
gPending = {
	--[[
	[dbnum] = {
		[mode] = { i insert  u update
			[collname] = {
				[id] = {data doc}
			}
		}
	}
	]]
}

function on_pending_save(dbnum)
	local info = gPending[dbnum] or {}
	--[[if info["i"] then]]
		--for collname, nodes in pairs(info["i"]) do
			--for id, node in pairs(info["i"][collname]) do
				--node._id = id
				--info["i"][collname][id] = nil
				--mongo:insertOne(dbnum,1,collname,node)
			--end
		--end
	--[[end]]
	if info["u"] then
		for collname, nodes in pairs(info["u"]) do
			for id, node in pairs(info["u"][collname]) do
				info["u"][collname][id] = nil
				mongo:updateMany(dbnum,1,collname,{_id = id},node)
			end
		end
	end
end

function do_pending_save()
	on_pending_save(0)
	on_pending_save(1)
end

function insert_db_save(dbnum,collname,id,k,v)
	gPending[dbnum] = gPending[dbnum] or {}
	gPending[dbnum]["u"] = gPending[dbnum]["u"] or {}
	gPending[dbnum]["u"][collname] = gPending[dbnum]["u"][collname] or {}
	gPending[dbnum]["u"][collname][id] = gPending[dbnum]["u"][collname][id] or {}
	local pend = gPending[dbnum]["u"][collname][id]
	pend[k] = v
end

function module_class(dbnum,module_name,template,collname)
	local mod = _G[module_name]
	local _mt = {
		__index = function(tab,k)
			if tab._pro[k] or tab._pro[k] == false then return tab._pro[k] end
			if template[k] then
				tab._pro[k] = resmng:deep_copy(template[k])
				return tab._pro[k]
			end
			return rawget(_G[module_name],k)
		end,
		__newindex = function(tab,k,v)
			if template[k] then
				if type(template[k]) ~= "table" then
					tab._pro[k] = v
					if v ~= template[k] then --简单类型值 不同才存
						insert_db_save(dbnum,collname,tab._id,k,v)
						return
					end
				else
					tab._pro[k] = v
					insert_db_save(dbnum,collname,tab._id,k,v)
				end
			else
				rawset(tab,k,v) --不在模板内则不入库
			end
		end
	}
	mod.wrap = function(node)
		return setmetatable({_pro = node}, _mt)
	end
	mod.new = function(node)
		if not node._id then LWARN("[MODULE_CLASS] new with no_id, node: %s",resmng:dump(node)) end
		local t = setmetatable({_pro = node}, _mt)
		return t
	end
end
