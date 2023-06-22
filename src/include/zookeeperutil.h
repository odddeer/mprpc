#pragma once

#include <semaphore.h> //信号量头文件
#include <zookeeper/zookeeper.h> //zk头文件
#include <string>

//封装的zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // zkclient启动连接zkserver
    void Start();
    // 在zkserver上根据指定的path创建znode节点
    void Create(const char* path,const char* data,int datalen,int state=0);
    // 根据参数指定的znode节点路径，获取znode节点的值
    std::string GetData(const char* path);
private:
    // zk的客户端句柄，类似对象指针，用来调用zk的功能
    zhandle_t *m_zhandle;//zhandle_t是一个结构体类型
};
