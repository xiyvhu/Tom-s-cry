//ID服务器模块
//声明服务器类

#pragma once

#include <lib_acl.hpp>

//服务器类
class server_c:public acl::master_threads {
protected:
    //进程启动时被调用
	void proc_on_init(void);
	
	//进程企图退出时被调用
	//返回 true 进程立即退出
	//若配置项中 ioctl_quick_abort非0，进程立即退出
	// 待所有客户机链接都关闭后，进程才退出  
	bool proc_exit_timer(size_t nclients,size_t nthreads);
		
	//线程获得连接时被调用，函数返回true，与线程绑定的连接将会保持，否则将被关闭
	bool thread_on_accept(acl::socket_stream* conn);
	
	//与线程绑定的连接可读时被调用函数返回true，与线程绑定的连接将会保持，否则将被关闭
	bool thread_on_read(acl::socket_stream* conn);
	
	//与线程绑定的连接发生读写超时被调用函数返回true，与线程绑定的连接将会保持，否则将被关闭
	bool thread_on_timeout(acl::socket stream* conn);
	
	//与线程绑定的连接被关闭时被调用
	void thread_on_close(acl::socket_stream* conn);
};