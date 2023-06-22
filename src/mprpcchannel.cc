#include "mprpcchannel.h"
#include <string>
#include "rpcheader.pb.h"

#include <sys/socket.h>
#include <sys/types.h>

#include "mprpcapplication.h"
#include <arpa/inet.h>
#include <unistd.h>

#include "mprpccontroller.h" //引入控制对象controller供提示过程中出现的问题，不是return结束了事
#include "zookeeperutil.h"

/*
header_size + service_name method_name args_size +args
 */

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                                    google::protobuf::RpcController* controller,
                                    const google::protobuf::Message* request,
                                    google::protobuf::Message* response,
                                    google::protobuf::Closure* done)
{
    // 发送请求的序列化
    const google::protobuf::ServiceDescriptor *sd = method->service();//得到请求的描述
    std::string service_name = sd->name(); // 得到service_name
    std::string method_name = method->name();// 得到method_name

    // 获取参数的序列化字符串长度 args_size
    int args_size=0;
    std::string args_str;
    if(request->SerializeToString(&args_str))
    {
        args_size=args_str.size();
    }
    else
    {
        //std::cout<<"serialize request error!"<<std::endl;
        controller->SetFailed("serialize request error!");
        return;
    }

    //定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size=0;
    std::string rpc_header_str;
    if(rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size=rpc_header_str.size();
    }
    else
    {
        //std::cout<<"serialize rpc header error!"<<std::endl;
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    //组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0,std::string((char*)&header_size,4));//从0开始插4个字节，即两位char header_size
    send_rpc_str+=rpc_header_str;//rpcheader
    send_rpc_str+=args_str;//args

    // 打印调试信息
    std::cout<<"************************************"<<std::endl;
    std::cout<<"header_size:"<<header_size<<std::endl;//16
    std::cout<<"rpc_header_str:"<<rpc_header_str<<std::endl;//UserServiceLoginzhang san123456
    std::cout<<"service_name:"<<service_name<<std::endl;//UserService
    std::cout<<"method_name:"<<method_name<<std::endl;//Login
    std::cout<<"args_str: "<<args_str<<std::endl;//zhang san123456
    std::cout<<"************************************"<<std::endl;

    // 使用tcp编程，完成rpc方法的远程调用
    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(-1==clientfd)
    {
        //std::cout<<"create socket error! errno:"<<errno<<std::endl;
        //exit(EXIT_FAILURE);
        char errtxt[512]={0};
        sprintf(errtxt,"create socket error! errno:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 读取配置文件rpcserver(远程提供服务的服务器)的信息
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    // 以上是通过配置文件读取的服务目的ip和port，以下为上zk来找目的ip和端口的代码
    ZkClient zkCli;
    zkCli.Start();
    //  /UserServiceRpc/Login
    std::string method_path="/"+service_name+"/"+method_name;
    // 通过zk获得服务地址 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    if(host_data=="")
    {
        controller->SetFailed(method_path+" is not exist!");
        return;
    }
    int idx=host_data.find(":");
    if(idx==-1)
    {
        controller->SetFailed(method_path+" address is invalid!");
        return;
    }
    std::string ip=host_data.substr(0,idx);
    uint16_t port= atoi(host_data.substr(idx+1,host_data.size()-idx).c_str());


    // 依据配置信息，填写socket结构体供后续绑定远程服务提供的ip
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port= htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc服务节点
    if(-1==connect(clientfd,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        //std::cout<<"connect socket error! errno:"<<errno<<std::endl;
        close(clientfd);
        //exit(EXIT_FAILURE);
        char errtxt[512]={0};
        sprintf(errtxt,"connect socket error! errno:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 发送rpc请求
    if(-1==send(clientfd,send_rpc_str.c_str(), send_rpc_str.size(),0))
    {
        //std::cout<<"send error! errno:"<<errno<<std::endl;
        close(clientfd);
        char errtxt[512]={0};
        sprintf(errtxt,"send error! errno:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 接收rpc请求的响应值
    char recv_buf[1024]={0};
    int recv_size = 0;
    if(-1 == (recv_size=recv(clientfd,recv_buf,1024,0)))
    {
        //std::cout<<"recv error! errno:"<<errno<<std::endl;
        close(clientfd);
        char errtxt[512]={0};
        sprintf(errtxt,"recv error! errno:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    //反序列化rpc调用的响应数据                                                              string 构造函数
    //std::string response_str(recv_buf,0,recv_size);// bug出现问题，recv_buf（"\n\000\020\001"）中遇到\0后面的数据就存不下来了。
    //if(!response->ParseFromString(response_str))//把远程发过来的回应解析给response
    if(!response->ParseFromArray(recv_buf,recv_size))
    {// 解析失败打印如下：
        //std::cout<<"parse error! response_str:"<<recv_buf<<std::endl;
        close(clientfd);
        char errtxt[512]={0};
        sprintf(errtxt,"parse error! response_str:%s",recv_buf);
        controller->SetFailed(errtxt);
        return;
    }

    close(clientfd);
}
