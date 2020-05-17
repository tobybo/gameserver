# gameserver
gameserver/linux/c++/lua/nginx/epoll/lua-intf

_viewlib:

    just for view. 方便查看api
    
include:

    头文件
    
app:

    服务器主逻辑
    
logic:

    游戏逻辑处理 算是枢纽层处理 承上(lua)启下(c++)
    
script:

    游戏具体上层业务 lua脚本: 支持收发数据，操作mongo，设置定时器
    
luaIntf:

    https://github.com/SteveKChiu/lua-intf 编译源码 方便调试
    
lualib:

    lua5.3src 编译源码 方便调试
    
misc:

    crc32校验 内存分配类 线程池 定时器
    
mongodb:

    mongo接口类: 根据配置自动连接，数据库操作
    
mysql:

    mysql连接池，操作接口类
    
net:

    网络连接相关: 监听和连接管理，收发tcp数据包处理
    
objects:

    c++层游戏对象管理类 现在没用了，具体的对象处理都交给lua层了
    
signal:

    信号相关 待完善，主要用于通过信号来操作游戏进程 例如 关服，reload脚本 等
    
tools:

    日志管理 master,worker 进程管理
    
server.conf

    服务器基本的配置文件 不是游戏业务逻辑相关的配置
    

