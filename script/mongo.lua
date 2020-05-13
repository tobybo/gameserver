mongo = {}
setmetatable(mongo,{__index = _G})
_ENV = mongo

reqId = reqId or 0

function incId(self)
	reqId = reqId + 1
	return reqId
end

function putDbCo(self,req_id)
	local co = coroutine.running()
	gMongoCo[req_id] = co
	return coroutine.yield()
end

function popDbCo(self,ret,req_id,opmod,res)
	local co = gMongoCo[req_id]
	if not co then return end
	gMongoCo[req_id] = nil
	local info = decode_from_mongo(res)
	local ret_bool = ret == 0 and true or false
	coroutine.resume(co,ret_bool,info)
end

function find(self,dbnum,collname,filter)
	req_id = incId(self)
	c_lib:flushMongoBuff(req_id,dbnum,1,0,collname,"")
	c_lib:writeDocument(encode(filter))
	c_lib:runCommandMongo()
	return putDbCo(self,req_id) --find 必须要等待回调
end

function insertOne(self,dbnum,nocallback,collname,doc)
	req_id = incId(self)
	c_lib:flushMongoBuff(req_id,dbnum,0,nocallback,collname,"")
	c_lib:writeDocument(encode(doc))
	c_lib:runCommandMongo()
	if nocallback ~= 1 then
		return putDbCo(self,req_id)
	end
end

function updateMany(self,dbnum,nocallback,collname,filter,update_doc)
	local update_node = {["$set"] = update_doc}
	req_id = incId(self)
	LERR("=========updateMany, update_val: %s",encode(update_node))
	c_lib:flushMongoBuff(req_id,dbnum,2,nocallback,collname,encode(update_node))
	c_lib:writeDocument(encode(filter))
	c_lib:runCommandMongo()
	if nocallback ~= 1 then
		return putDbCo(self,req_id)
	end
end


