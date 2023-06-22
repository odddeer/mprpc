#pragma once
#include <google/protobuf/service.h>
//#include <memory> //智能指针头文件
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>



//框架提供的专门发布rpc服务的网络对象类
class RpcProvider
{
public:
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

private:
    /*
    // 组合了TcpServer 只会在该类中使用，直接丢到run方法里当个局部变量即可
    std::unique_ptr<muduo::net::TcpServer> m_tcpserverPtr;//指向tcpserver对象，用于bind,epoll树、ip\port
    */
    // 组合了eventLoop
    muduo::net::EventLoop m_eventLoop;//相当于创建epoll对象，用于内核监听

    struct ServiceInfo
    {
        google::protobuf::Service *m_service;//保存服务机对象
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap;//保存服务方法名和其方法描述映射表
    };
    //存储注册成功的服务对象(服务提供者)和其服务方法的所有信息 <服务名，服务名-<服务旗下方法名,方法描述>>
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    //lfd 的回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    //cfd 的回调
    void OnMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp);
    // Closure返回的回调操作,用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};
