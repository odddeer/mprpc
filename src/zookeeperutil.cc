#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>

// 全局的watcher观察器 zkserver给zkclient的通知
void global_watcher(zhandle_t *zh,int type, //type代表回调得到的消息是哪种类型
                    int state,const char* path,void *watcherCtx)
                    // state代表连接的状态是什么
{
    //zh句柄在init为主线程，自动用poll监听server回信为I/O线程，这里解析到回信的东西设置信号量是回调线程
    if(type==ZOO_SESSION_EVENT)//回调的消息类型是和会话相关的消息类型（该类型包括成功和失败两个宏）
    {
        if(state==ZOO_CONNECTED_STATE)//连接状态为连接成功
        {
            sem_t *sem =(sem_t*) zoo_get_context(zh);//把指定句柄zh连接连接的信号量拿出来
            sem_post(sem);//给该信号量赋值为1，代表连接成功
        }
    }
}

//zk客户端构造函数
ZkClient::ZkClient():m_zhandle(nullptr)
{

}

//zk客户端析构函数
ZkClient::~ZkClient()
{
    if(m_zhandle!= nullptr)
    {
        zookeeper_close(m_zhandle);//关闭句柄，释放资源
    }
}

//连接zkserver（异步）
void ZkClient::Start()
{
    //通过调用应用提供类对象，调用其获取配置文件方法，再调用其load配置map的方法，获得zkserver的ip和port
    std::string host=MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port=MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr=host+":"+port;//127.0.0.1:zookeeperport=2181
    /*
        zookeeper_mt多线程版本下的init
        该init的api客户端提供了三个线程：
           API调用线程（init所在函数的线程）
           网络I/O线程 pthread_create 用poll实现的监听socket建立连接
           watcher回调函数线程，即下面的global_watcher回调函数线程，通过标志位结束init的wait等待完成一个流程
    */
                            //指定如上格式的ip+port输入 变化后激起的回调函数  会话的超时时间30s
    m_zhandle = zookeeper_init(connstr.c_str(),global_watcher,30000, nullptr, nullptr,0);
    if(nullptr==m_zhandle)
    {//这里主线程只是检查创建句柄是否成功，不代表连接成功
        std::cout<<"zookeeper_init error!"<<std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;//创建一个信号量（中断）
    sem_init(&sem,0,0);//初始化设置信号量为0
    zoo_set_context(m_zhandle,&sem);//给句柄添加信息，给句柄绑定信号量

    sem_wait(&sem);//中断为0接着阻塞，为1就完成改行，代表客户端建立连接成功
    std::cout<<"zookeeper_init success!"<<std::endl;
}
               //创建znode 节点路径          对应内容数值       内容长度        永久节点还是临时节点（没心跳就消除掉）
 void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen=sizeof(path_buffer);
    int flag;
    //先判断，查看你指定的path在server中是否已经存在
    flag= zoo_exists(m_zhandle,path,0, nullptr);
    if(ZNONODE==flag)//创建的节点并不存在
    {
        //创建指定path的节点 通过m_zhandle来实现与server的交互调用
        flag= zoo_create(m_zhandle,path,data,datalen,
                         &ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen);
        if(flag==ZOK)//创建成功
        {
            std::cout<<"znode create success... path:"<<path<<std::endl;
        }
        else
        {
            std::cout<<"flag:"<<flag<<std::endl;
            std::cout<<"znode create error... path:"<<path<<std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

//根据指定的path,获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen=sizeof(buffer);        //把节点内容放在buffer里
    int flag= zoo_get(m_zhandle,path,0,buffer,&bufferlen, nullptr);
    if(flag!=ZOK)
    {
        std::cout<<"get znode error... path:"<<path<<std::endl;
        return "";
    }
    else
    {
        return buffer;
    }
}
