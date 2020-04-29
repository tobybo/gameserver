1.玩家账号的密码要存md5值.     over
2.是否mysql和mongodb 同时使用. over 优先mongodb
3.收消息接到lua端处理.         over
4.发消息接到lua端处理.
5.lua_loop 放到c++端.
6.服务器定帧逻辑.
7.服务器定时器.
8.接入mongodb.
...


延申:
1.考虑服务器doctor,k8s等技术.


bug:
1.发了错误包后再发送包会无法处理，即时是新的连接。
