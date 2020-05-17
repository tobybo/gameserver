1.玩家账号的密码要存md5值.     over
2.是否mysql和mongodb 同时使用. over 优先mongodb
3.收消息接到lua端处理.         over
4.发消息接到lua端处理.         over
5.lua_loop 放到c++端.          over
6.服务器定帧逻辑.              over
7.服务器定时器.                over
8.接入mongodb.                 over
9.内存管理.
10.to_json接口添加.            over
11.心跳包.
12.短时间单个连接收消息数量限制.
13.总连接数限制.
...

延申:
1.考虑服务器doctor,k8s等技术.


bug:
1.发了错误包后再发送包会无法处理，即使是新的连接.
	已解决：错误包没有及时丢弃导致死锁.
2.有些互斥量没有destroy
