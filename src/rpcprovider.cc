#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"

#include "logger.h"
#include "zookeeperutil.h"

// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    //获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    //获取服务的名字
    std::string service_name = pserviceDesc->name();//解析的是框架服务名UserServiceRpc，而不是服务机本地名UserService
    //获取服务对象service的方法的数量 如 login就是一个
    int methodCnt=pserviceDesc->method_count();

    //std::cout<<"service_name:"<<service_name<<std::endl;
    LOG_INFO("service_name:%s",service_name.c_str());

    for(int i=0;i<methodCnt;++i)
    {
        //获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name,pmethodDesc});
        //std::cout<<"method_name:"<<method_name<<std::endl;
        LOG_INFO("method_name:%s",method_name.c_str());
    }
    service_info.m_service=service;
    m_serviceMap.insert({service_name,service_info});
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");
    // 绑定连接回调回调方法
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    // 绑定消息读写回调方法
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1,
                                        std::placeholders::_2,std::placeholders::_3));

    //设置muduo库的线程数量 (epoll+多线程) 分离业务代码和网络代码
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // 会话时间 timeout  30s zkclient 网络I/O线程 1/3*timeout时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();//开始连接zk
    //把service_name、method_name在zk上建立节点
    for(auto &sp:m_serviceMap)
    {
        // service_name 设为永久节点 服务路径：/service_name 如：/UserServiceRpc
        std::string service_path="/"+sp.first;                                             //znode永久节点
        zkCli.Create(service_path.c_str(), nullptr,  0);
        for(auto &mp:sp.second.m_methodMap)
        {
            // /service_name/method_name 如：/UserServiceRpc/Login
            std::string method_path = service_path+"/"+mp.first;
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);                                     // znode临时节点
            zkCli.Create(method_path.c_str(),method_path_data, strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }

    // rpc服务端准备启动，打印信息
    std::cout<<"RpcProvider start service at ip:"<<ip<<" port:"<<port<<std::endl;

    //启动网络服务
    server.start();// listen epoll_ctl添加到epoll上
    m_eventLoop.loop();//epoll_wait以阻塞的方式等待新用户连接，已连接用户的读写事件等，然后回调onConnection和onMessage方法
}

// lfd的回调 socket连接
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {
        // 和 rpc client的连接断开
        conn->shutdown();
    }
}

/*
    在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
    service_name method_name args 定义proto的message类型，进行数据头的序列化和反序列化
                                  service_name method_name args_size
    16UserServiceLoginzhang san123456
    header_size(4个字节) + header_str + args_str 用protobuf来解析
 */

// 已建立连接用户的读写事件回调 如果远程有一个rpc服务的调用请求，那么OnMessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer* buffer,muduo::Timestamp)
{
    //网络上接收的远程rpc调用请求的字符流 Login args
    std::string recv_buf = buffer->retrieveAllAsString();

    //从字符流中读取前4个字节的内容 char[i]占2字节 两个就4字节
    uint32_t header_size=0;
    recv_buf.copy((char*)&header_size,4,0);

    // 根据header_size读取数据头的原始字符流, 反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str=recv_buf.substr(4,header_size);//service_name method_name args_size
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name=rpcHeader.service_name();
        method_name=rpcHeader.method_name();
        args_size=rpcHeader.args_size();
    }
    else
    {
        //数据头反序列化失败
        std::cout<<"rpc_header_str:"<<rpc_header_str<<" parse error!"<<std::endl;
        return;
    }

    //获取rpc方法参数的字符流数据
    std::string args_str=recv_buf.substr(4+header_size,args_size);

    // 打印调试信息
    std::cout<<"************************************"<<std::endl;
    std::cout<<"header_size:"<<header_size<<std::endl;//16
    std::cout<<"rpc_header_str:"<<rpc_header_str<<std::endl;//UserServiceLoginzhang san123456
    std::cout<<"service_name:"<<service_name<<std::endl;//UserService
    std::cout<<"method_name:"<<method_name<<std::endl;//Login
    std::cout<<"args_str:"<<args_str<<std::endl;//zhang san123456
    std::cout<<"************************************"<<std::endl;

    // 获取service对象和method对象
    auto it=m_serviceMap.find(service_name);//找服务器上服务类UserService
    if(it==m_serviceMap.end())//没有找到提供的服务对象
    {
        std::cout<<service_name<<" is not exist!"<<std::endl;
        return;
    }

    auto mit=it->second.m_methodMap.find(method_name);//找UserService对象的方法
    if(mit==it->second.m_methodMap.end())
    {
        std::cout<<service_name<<":"<<method_name<<" is not exist!"<<std::endl;
        return;
    }

    //    second:ServiceInfo {服务对象，方法名-方法描述map}
    google::protobuf::Service *service=it->second.m_service;//获取UserService的new对象
    const google::protobuf::MethodDescriptor *method=mit->second;//获取对象方法的方法描述

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request=service->GetRequestPrototype(method).New();//依方法描述，获得方法参数
    if(!request->ParseFromString(args_str))
    {//参数解析失败，返回
        std::cout<<"request parse error, content:"<<args_str<<std::endl;
        return;
    }
    google::protobuf::Message *response=service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用，绑定一个Closure的回调函数 , 生成一个回调done
    google::protobuf::Closure *done=google::protobuf::NewCallback<RpcProvider,
                                                const muduo::net::TcpConnectionPtr&,
                                                google::protobuf::Message*>
                                                (this,
                                                 &RpcProvider::SendRpcResponse,
                                                 conn,response);

    // 在框架上根据远端rpc请求，调用当前rpc节点(服务提供者)发布的方法
    // new UserService().Login(controller,request,response,done) 实际调的这个，框架如下
    service->CallMethod(method, nullptr, request, response, done);
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response)
{
    std::string response_str;
    if(response->SerializePartialToString(&response_str))
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送给rpc的调用方
        conn->send(response_str);
    }
    else
    {
        std::cout<<"serialize response_str error!"<<std::endl;
    }
    conn->shutdown();// 模拟http的短链接服务，由rpcprovider主动断开连接
}