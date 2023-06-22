#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"

int main(int argc,char** argv)
{
    //整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);

    //演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());//stub消费端要传递Channel对象来调用
    //rpc服务所要提供的参数 输入
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    //rpc服务返回的结果接收
    fixbug::LoginResponse response;
    // 发起远程rpc方法的调用 阻塞等待远程rpc执行结果的返回
    stub.Login(nullptr,&request,&response, nullptr);//调用无论哪种方法都是 MprpcChannel::callmethod方法
    // 底层都是 RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络化

    //一次rpc调用完成，读调用的结构
    if(0==response.result().errcode())
    {
        std::cout<<"rpc login response success: "<<response.success()<<std::endl;
    }
    else
    {
        std::cout<<"rpc login response error: "<<response.result().errmsg()<<std::endl;
    }

    // 演示调用远程发布的rpc方法Register
    // a.创建申请服务对象，填写要请求服务所需参数
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("666666");
    // b.创建服务接收对象，供后续服务完毕，结果的回传
    fixbug::RegisterResponse rsp;

    // c.以同步的方式发起rpc调用请求，等待返回结果(阻塞等待)
    stub.Register(nullptr,&req,&rsp, nullptr);

    // d.调用结果判断打印
    if(0==rsp.result().errcode())
    {
        std::cout<<"rpc register response success: "<<response.success()<<std::endl;
    }
    else
    {
        std::cout<<"rpc register response error: "<<response.result().errmsg()<<std::endl;
    }

    return 0;
}
