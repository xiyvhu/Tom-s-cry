//跟踪服务器模块
//定义缓存类

#include "01_globals.h"
#include "03_cache.h"

//根据键获取值
int cache_c::get(char* const key,acl::string& value)const{
	//构造键
	acl::string tracker_key;
	tracker_key.format("%s:%s",TRACKER_REDIS_PREFIX,key);//添加Redis前缀，避免键冲突
	
	//检查Redis连接池
	if(!g_rconns){
		logger_warn("redis connection pool is null,key:%s",tracker_key.c_str())；
		return ERROR;
	}
	
	//从连接池中获取一个Redis连接
	acl::redis_client* rconn = (acl::redis_client*)g_rconns->peek();
	if(!rconn){
		logger_warn("peek redis connection fail,key:%s",tracker_key.c_str());
		return ERROR;
	}
	
	//持有此连接的Redis对象即为Redis客户机
	acl::redis redis;
	redis.set_client(rconn);
	
	//借助Redis客户机根据键获取值
	if(!redis.get(tracker_key.c_str(),value)){
		logger_warn("get cache fail,key:%s",tracker_key.c_str());
		g_rconns->put(rconn,false);
		return ERROR;
	}
	
	//检查空值
	if(value.empty()){
		logger_warn("value is empty,key:%s",tracker_key.c_str());
		g_rconns->put(rconn,false);
		return ERROR;
	}
	
	//将连接重新标记为空闲
	logger("get cache ok,key:%s,value:%S",tracker_key.c_str(),value.c_str());
	g_conns->put(rconn,true);
	
	return OK;	
}
	
//设置置顶键的值
int cache_c::set(char* const key,char const* value,int timeout /* = -1 */)const{
	//构造键
	acl::string tracker_key;
	tracker_key.format("%s:%s",TRACKER_REDIS_PREFIX,key);//添加Redis前缀，避免键冲突
	
	//检查Redis连接池
	if(!g_rconns){
		logger_warn("redis connection pool is null,key:%s",tracker_key.c_str())；
		return ERROR;
	}
	
	//从连接池中获取一个Redis连接
	acl::redis_client* rconn = (acl::redis_client*)g_rconns->peek();
	if(!rconn){
		logger_warn("peek redis connection fail,key:%s",tracker_key.c_str());
		return ERROR;
	}
	
	//持有此连接的Redis对象即为Redis客户机
	acl::redis redis;
	redis.set_client(rconn);
	
	//借助Redis客户机设置指定键的值
	if(timeout < 0){
		timeout = cfg_ktimeout;
	}
	if(!redis.setex(tracke_key.c_str(),value,timeout)){
		logger_warn("set cache fail,key%s,value:%s,timeout:%d",tracker_key.c_str(),value,timeout);
		g_rconns->put(rconn,false);
		return ERROR;
	}

	//将连接重新标记为空闲
	logger("set cache ok,key:%s,value:%S,timeout:%d",tracker_key.c_str(),value,timeout);
	g_conns->put(rconn,true);
	
	return OK;
}
	
//删除指定键的值
int cache_c::del(char const* key)const{
	//构造键
	acl::string tracker_key;
	tracker_key.format("%s:%s",TRACKER_REDIS_PREFIX,key);//添加Redis前缀，避免键冲突
	
	//检查Redis连接池
	if(!g_rconns){
		logger_warn("redis connection pool is null,key:%s",tracker_key.c_str())；
		return ERROR;
	}
	
	//从连接池中获取一个Redis连接
	acl::redis_client* rconn = (acl::redis_client*)g_rconns->peek();
	if(!rconn){
		logger_warn("peek redis connection fail,key:%s",tracker_key.c_str());
		return ERROR;
	}
	
	//持有此连接的Redis对象即为Redis客户机
	acl::redis redis;
	redis.set_client(rconn);
	
	//借助Redis客户机删除指定键值对
	if(!redis.del_one(tracker_key.c_str())){
		logger_warn("delete cache fail,key:%s",tracker_key.c_str());
		g_rconns->put(rconn,false);
		return ERROR;
	}
	
	//将连接重新标记为空闲
	logger("get cache ok,key:%s",tracker_key.c_str());
	g_conns->put(rconn,true);
	
	return OK;
}