//ID服务器模块
//实现服务器类

#include <unistd.h>
#include "02_proto.h"
#include "03_util.h"
#include "01_globals.h"
#include "05_service.h"
#include "07_server.h"

//进程启动时被调用
void server_c::proc_on_init(void){
	//MySQL地址表
	if(cfg_maddrs || !strlen(cfg_maddrs)){
		logger_fatal("mysql addresses is null");
	}
	split(cfg_maddrs,g_maddrs);
	if(g_maddrs.empty()){
		logger_fatal("mysql addresses is empty");
	}
	
	//主机名
	char hostname[256 + 1] = {};
	if(gethostname(hostname,sizeof(hostname) - 1)){
		logger_error("call gethostname fail:%s",strerror(errno));
	}
	g_hostname = hostname;

    //最大偏移
    if(cfg_maxconns < 10){
        logger_fatal("invalid maximum offset:%d < 10",cfg_maxoffset);
    }
	
	//打印配置信息
	logger("cfg_maddrs:%s,cfg_mtimeout:%d,cfg_maxoffset:%d",cfg_maddrs,cfg_mtimeout,cfg_maxoffset);	
}
	
//进程企图退出时被调用 
bool server_c::proc_exit_timer(size_t nclients,size_t nthreads){
	if(!nclients || !nthreads){
		logger("nclients:%lu,nthreads:%lu",nclients,nthreads);
		return true;
	}
	return false;
}	

//线程获得连接时被调用，函数返回true，与线程绑定的连接将会保持，否则将被关闭
bool server_c::thread_on_accept(acl::socket_stream* conn){
	logger("connect,from:%s",conn->get_peer());
	return true;
}
	
//与线程绑定的连接可读时被调用,函数返回true，与线程绑定的连接将会保持，否则将被关闭
bool server_c::thread_on_read(acl::socket_stream* conn){
	//接受包头
	char head[HEADLEN];
	if(conn->read(head,HEADLEN) < 0){
		if(conn->eog()){
			logger("connection has been closed,from:%s",conn->get_peer());
		}else{
			logger_error("read fail:%s,from:%s",acl::last_serror(),conn->get_peer());
		}
		return false;
	}
	
	//业务处理
	service_c service;
	return service.business(conn,head);
}
	
//与线程绑定的连接发生读写超时被调用,函数返回true，与线程绑定的连接将会保持，否则将被关闭
bool server_c::thread_on_timeout(acl::socket stream* conn){
	logger("read timeout,from:%s",conn->get_peer());
	return true;
}
	
//与线程绑定的连接被关闭时被调用
void server_c::thread_on_close(acl::socket_stream* conn){
	logger("client disconnect,from:%s",conn->get_peer());
}