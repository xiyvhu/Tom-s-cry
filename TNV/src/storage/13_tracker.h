//存储服务器模块
//声明跟踪客户机线程

#pragma once

#include <lib_acl.hpp>

//跟踪客户机线程
class tracker_c:public acl::thread{
public:
    //构造函数
    tracker_c(char const* taddr);

    //终止函数
    void stop(void);

protected:
    //线程过程函数
    void* run(void);

private:
    //向跟踪服务器发送加入包
    int join(acl::socket_stream* conn)const;

    //向跟踪服务器发送心跳包
    int beat(acl::socket_stream* conn)const;

    bool        m_stop; //是否终止
    acl::string m_taddr;//跟踪服务器地址
};