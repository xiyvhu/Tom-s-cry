//跟踪服务器模块
//定义业务服务类

#include <algorithm>
#include "01_globals.h"
#include "02_proto.h"
#include "03_util.h"
#include "05_db.h"
#include "07_service.h"

//业务处理
bool service_c::business(acl::socket_stream* conn,char* const head)const{
	//解析包头
	long long bodylen = ntoll(head); 				//包体长度
	if(bodylen < 0){
		error(conn,-1,"invalid body length:%lld < 0",bodylen);
	}
	int command = head[BODYLEN_SIZE];				//命令
	int status = head[BODYLEN_SIZE + COMMAND_SIZE]；//状态
	logger("bodylen:%lld,command:%d,status:%d",bodylen,command,status);
	
	bool result;
	//根据命令执行具体业务处理
	switch(comamnd){
		//处理来自存储服务器的加入包
		case CMD_TRACKER_JOIN:
			result = join(conn,bodylen);
			break;
			
		//处理来自存储服务器的心跳包
		case CMD_TRACKER_BEAT:
			result = beat(conn,bodylen);
			break;
			
		//处理来自客户机的获取存储服务器地址列表请求
		case CMD_TRACKER_SADDRS:
			result = saddrs(conn,bodylen);
			break;
			
		//处理来自客户机的获取组列表请求
		case CMD_TRACKER_GROUPS:
			result = groups(conn);
			break;
			
		default:
			error(conn,-1,"unknown command:%d",command);
			return false;
	}
	
	return result;
}

///////////////////////////////////////////////////////////////////////////////

//处理来自存储服务器的加入包
bool service_c::join(acl::socket_stream* conn,long long bodylen)const{
	//检查包体长度
	long long expected = sizeof(storage_join_body_t);//期望包体长度
	if(bodylen != expected){
		error(conn,-1,"invalid body length:%lld != %lld",bodylen,expected);
		return false;
	}
	
	//接收包体
	char body[bodylen];
	if(conn->read(body,bodylen) < 0){
		logger_error("read fail:%s,bodylen:%lld,from:%s",acl::last_serror(),bodylen,conn->get_peer());
		return false;
	}
	//解析包体
	storage_join_t sj;
	storage_join_body_t* sjb = (storage_join_body_t*)body;
	
	//版本
	strcpy(sj.sj_version,sjb->sjb->sjb_version);
	
	//组名
	strcpy(sj,sj_groupname,sjb->sjb_groupname);
	if(valid(sj.sj_groupname) != OK){//验证组名中是否包含非法字符
		error(conn,-1,"invalid groupname:%s",sj.sj_groupname);
		return false;
	}
	
	//主机名
	strcpy(sj.sj_hostname,sjb->sjb_hostname);
	
	//端口号
	sj.sj_port = ntos(sjb->sjb_port);
	if(!sj.sj_port){
		error(conn,-1,"invalid port:%u",sj.sj_port);
		return false;	
	}
	
	//启动时间
	sj.sj_stime = ntol(sjb->sjb_stime);
	
	//加入时间
	sj.sj_jtime = ntol(sjb->sjb_jtime);
	
	logger("storage join,version:%s,groupname:%s,hostname:%s,port:%u,stime:%s,jtime:%s",
			sj.sj_version,sj.sj_groupname,sj.sj_hostname,sj.sj_port,
			std::string(ctime(&sj.sj_stime)).c_str(),
			std::string(ctime(&sj.sj_jtime)).c_str());//ctime函数返回时间用的是个局部的静态变量，直接返回的话后值会覆盖前值
	
	//将存储服务器加入组表
	if(join(&sj,conn->get_peer()) != OK){
		error(conn,-1,"join into groups fail");
		return false;
	}
	
	return ok(conn);
}
	
//处理来自存储服务器的心跳包
bool service_c::beat(acl::socket_stream* conn,long long bodylen)const{
	//检查包体长度
	long long expected = sizeof(storage_beat_body_t);//期望包体长度
	if(bodylen != expected){
		error(conn,-1,"invalid body length:%lld != %lld",bodylen,expected);
		return false;
	}
	
	//接收包体
	char body[bodylen];
	if(conn->read(body,bodylen) < 0){
		logger_error("read fail:%s,bodylen:%lld,from:%s",acl::last_serror(),bodylen,conn->get_peer());
		return false;
	}

	//解析包体
	storage_beat_body_t* sbb = (storage_beat_body_t*)body;
	
	//组名
	char groupname[STORAGE_GROUPNAME_MAX + 1];
	strcpy(groupname,sbb->sbb_groupnam);

	//主机名
	char hostname[STORAGE_HOSTNAME_MAX + 1];
	strcpy(hostname,sbb->sbb_hostname);
	
	logger("storage beat,groupname:%s,hostname:%s",groupname,hostname);
	
	//将存储服务器标记为活动
	if(beat(groupname,hostname,conn->get_peer()) != OK){
		error(conn.-1,"mark storage as active fail");
		return false;
	}
	
	return ok(conn);
}
	
//处理来自客户机的获取存储服务器地址列表请求
bool service_c::saddrs(acl::socket_stream* conn,long long bodylen)const{
	//检查包体长度
	long long expected = APPID_SIZE + USERID_SIZE + FILEID_SIZE;
	if(bodylen != expected){
		error(conn,-1,"invalid body length:%lld != %lld",bodylen,expected);
		return false;
	}
	
	//接收包体
	char body[bodylen];
	if(conn->read(body,bodylen) < 0){
		logger_error("read fail:%s,bodylen:&lld,from:%s",acl::last_serror(),bodylen,conn->peer());
		return false;
	}
	
	//解析包体
	char appid[APPID_SIZE];
	strcpy(appid,body);
	char userid[USERID_SIZE];
	strcpy(userid,body + APPID_SIZE);
	char fileid[FILEID_SIZE];
	strcpy(fileid,body + APPID_SIZE + USERID_SIZE);
	
	//响应客户机存储服务器地址列表
	if(saddrs(conn,appid,userid) != OK){
		return false;//saddrs函数或打印出错日志
	}
	
	return true;
}
	
//处理来自客户机的获取组列表请求
bool service_c::groups(acl::socket_stream* conn)const{
	//互斥锁枷锁
	if(errno = pthread_mutex_lock(&g_mutex)){
		logger_error("call pthread_mutex_lock fail:%s",strerror(errno));
		return false;
	}
	
	//全组字符串
	acl::string gps;
	gps.format(
"COUNT OF GROUPS:%lu\n",g_groups.size());
	
	//遍历组表中的每一个组
	for(std::map<std::string,std::list<storage_info_t>>::const_iterator group = g_groups.begin();group != g_groups.end();++group){
		
		//单组字符串
		acl::string grp;
		grp.format(
"GROUPNAME:%s\n"
"GROUPNAME OF STORAGE:%lu\n"
"COUNT OF ACTIVE STORAGES:%s\n",
		group->first.c_str(),
		group->second.size(),
		"%d");
	
		int act = 0;//统计活动存储服务器数量
	
		//遍历该组中每一台存储服务器
		for(sstd::list<storage_info_t>::const_iterator si = group->second.begin();si != group->second.end();++si){
			acl::string stg;//存储服务器字符串
			stg.format(
"VERSION:%s\n"
"HOSTNAME:%s\n"
"ADDRESS:%s:%u\n"
"STARTUP TIME:%s\n"
"JOIN TIME:%s\n"
"BEAT TIME:%s\n"
"STATUS: ",
			si->si_version,
			si->si_hostname,
			si->si_addr,si->si_port,
			std::string(ctime(&si->si_stime)).c_str(),
			std::string(ctime(&si->si_jtime)).c_str(),
			std::string(ctime(&si->si_btime)).c_str());
		
			switch(si->si_status){
				case STORAGE_STATUS_OFFLINE:
					stg += "OFFLINE";
					break;
				case STORAGE_STATUS_ONLINE:
					stg += "ONLINE";
					break;
				case STORAGE_STATUS_ACTIVE:
					stg += "ACTIVE";
					++act;
					break;
				default:
					std += "UNKNOWN";
					break;
			}
			grp += stg + "\n";;
		}
		gps += grp.format(grp,act);
	}
	//去掉最后一换行符
	gps = gps.left(gps.size() - 1);
	
	//互斥锁解锁
	if(errno = pthread_mutex_unlock(&g_mutex)){
		logger_error("call pthread_mutex_unlock fail:%s",strerror(errno));
		return false;
	}
	
	//构造响应
	long long bodylen = gps.size() + 1;
	long long resplen = HEADLEN + bodylen;
	char resp[resplen] = {};
	llton(bodylen,resp);
	resp[BODYLEN_SIZE] = CMD_TRACKER_REPLY;
	resp[BODYLEN_SIZE + COMMAND_SIZE] = 0;
	strcpy(resp + HEADLEN,gps.c_str());
	
	//发送响应
	if(conn->write(resp,resplen) < 0){
		logger_error("write fail:%s,resplen:%lld,to:%s",acl::last_serror(),resplen,conn->get_peer());
		return false;
	}
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////	
	
//将存储服务器加入组表
int service_c::join(storage_join_t const* sj,char const* saddr)const{
	
	return OK;
}
	
//将存储服务器标记为活动
int service_c::beat(char const* groupname,char const* hostname,char const* saddr)const{
	
	return OK;
}
	
//响应客户机存储服务器地址列表
int service_c::saddrs(acl::socket_stream* conn,char const* appid,char const* userid)const{
	
	return OK;
}
	
//根据用户ID获取其对应组名
int service_c::group_of_user(char const* appid,char const* userid,std::string& groupname)const{
	
	return OK;
}
	
//根据组名获取存储服务器的地址列表
int service_c::saddrs_of_group(char const* groupname,std::string& saddrs)const{
	
	return OK;
}
	
///////////////////////////////////////////////////////////////////////////////	
	
//应答成功
bool service_c::ok(acl::socket_stream* conn)const{
	
	return true;
}
	
//应答失败
bool service_c::error(acl::socket_stream* conn,short errnumb,char const* format,...)const{
	
	return true;
}