//存储服务器模块
//定义ID客户机类

#include <lib_acl.hpp>
#include "02_proto.h"
#include "03_util.h"
#include "01_globals.h"
#include "09_id.h"

//从ID服务器获取与ID键相对应的值
long id_c::get(char const* key)const{
    //检查ID键
    if(!key){
        logger_error("key is null");
        return -1;
   }
   size_t keylen = strlen(key);
   if(!keylen){
        logger_error("key is null");
        return -1'
   }
   if(keylen > ID_KEY_MAX){
        logger_error("key too big:%lu > %d",keylen,ID_KEY_MAX);
        return -1;
   }

    //构造请求
    long long bodylen = keylen + 1;
    long long requlen = HEADLEN + bodylen;
    char requ[requlen] = {};
    llton(bodylen,requ);
    requ[BODYLEN_SIZE] = CMD_ID_GET;
    requ[BODYLEN_SIZE + COMMAND_SZIE] = 0;
    strcpy(requ + HEADLEN,key);

    //发送请求
    return client(requ,requlen);
}

//向ID服务器发送请求，接受并解析相应，从中获取ID值
long ic_c::client(char const* requ,long long requlen)const{
    //从ID服务器地址表中随机抽取一台ID服务器并尝试连接
    acl::socket_stream conn;

    //向ID服务器发送请求
    srand(time(NULL));
    int nids = g_iaddrs.szie();
    int nrand = rand() % nids;
    for(int i = 0;i < nids;++i){
        if(conn.open(g_iaddrs[nrand].c_str(),0,0)){
            logger("connect id success,addr:%s",g_iaddrs[nrand].c_str());
            break;
        }else{
            logger_error("connect id fail,addr:%s",g_iaddrs[nrand].c_str());
            nrand = (nrand + 1) % nids;
        }
    }
    if(conn.alive()){
        logger_error("connect id fail.addr:%s",cfg_iaddrs);
        return -1;
    }

    //从ID服务器接收响应
    long long resplen = HEADLEN + BODYLEN_SIZE;
    char resp[resplen] = {};
    if(conn.read(resp,resplen) < 0){
        logger_error("call read fail:%s,resplen:%lld,from:%s",acl::last_serror(),resplen,conn.get_peer());
        conn.close();
        return -1;
    }

    //从ID服务器的响应中解析出ID值
    long value = ntoll(resp + HEADLEN);

    conn.close();

    return value;
}