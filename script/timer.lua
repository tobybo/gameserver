-- 定时器相关
-- addtimer: timer:addTimer(timer_id,p1,p2,p3,p4)
-- call_back: handlerTimer(timer_id,p1,p2,p3,p4)
timer = {}
setmetatable(timer,{__index = _G})
_ENV = timer

function addTimer(self,msec,timer_id,p1,p2,p3,p4)
	if not TIMER_BACK_FUNS[timer_id] then
		LERR("[TIMER] addTimer err, no timer_call_back, timer_id: %d",timer_id)
		return
	end
	log("[TIMER] addTimer, msec: %d, timer_id: %d",msec,timer_id)
	c_lib:addTimer(msec,timer_id,p1 or 0,p2 or 0,p3 or "",p4 or "")
end

function handlerTimer(self,timer_id,p1,p2,p3,p4)
	local func = TIMER_BACK_FUNS[timer_id]
	if not func then
		LERR("[TIMER] addTimer err, no timer_call_back, timer_id: %d",timer_id)
		return
	end
	func(p1,p2,p3,p4)
end

-------------------
first_one = 0
local function next_one()
	first_one = first_one + 1
	return first_one
end

TIMER_BACK_FUNS = {}

TIMER_TEST_FUN1 = next_one()
TIMER_TEST_FUN2 = next_one()
TIMER_TEST_FUN3 = next_one()
TIMER_TEST_FUN4 = next_one()
TIMER_TEST_FUN5 = next_one()

TIMER_BACK_FUNS[TIMER_TEST_FUN1] = function(p1,p2,p3,p4)
	LLOG("[TIMER] this is timer back fun, id: %d, p1: %d, p2: %d, p3: %s, p4: %s",
		TIMER_TEST_FUN1,p1,p2,p3,p4)
end

TIMER_BACK_FUNS[TIMER_TEST_FUN2] = function(p1,p2,p3,p4)
	LLOG("[TIMER] this is timer back fun, id: %d, p1: %d, p2: %d, p3: %s, p4: %s",
		TIMER_TEST_FUN2,p1,p2,p3,p4)
end

TIMER_BACK_FUNS[TIMER_TEST_FUN3] = function(p1,p2,p3,p4)
	LLOG("[TIMER] this is timer back fun, id: %d, p1: %d, p2: %d, p3: %s, p4: %s",
		TIMER_TEST_FUN3,p1,p2,p3,p4)
end

TIMER_BACK_FUNS[TIMER_TEST_FUN4] = function(p1,p2,p3,p4)
	LLOG("[TIMER] this is timer back fun, id: %d, p1: %d, p2: %d, p3: %s, p4: %s",
		TIMER_TEST_FUN4,p1,p2,p3,p4)
end

TIMER_BACK_FUNS[TIMER_TEST_FUN5] = function(p1,p2,p3,p4)
	LLOG("[TIMER] this is timer back fun, id: %d, p1: %d, p2: %d, p3: %s, p4: %s",
		TIMER_TEST_FUN5,p1,p2,p3,p4)
end


