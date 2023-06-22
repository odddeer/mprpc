//
// Created by lenovo on 2023/6/8.
//
#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

/*
    UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendLists方法
*/
class UserService : public fixbug::UserServiceRpc//继承框架 发布服务端
{
public:
    // 本地方法实现：
    bool Login(std::string name, std::string pwd)
    {
        std::cout<<"doing local service: Login"<<std::endl;
        std::cout<<"name:"<< name <<"pwd:"<< pwd <<std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout<<"doing local service: Register"<<std::endl;
        std::cout<<"id:"<<id<<" name:"<<name<<" pwd:"<<pwd<<std::endl;
        return true;
    }

    /*
       重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
       1.caller ===> Login(LoginRequest)申请 ==> muduo传递到服务机 ==> callee
       2.callee ===> Login(LoginRequest)得到反序列化数据 ==> 交到下面重写的这个Login方法上了
     */

    void Login(::google::protobuf::RpcController* controller,
               const ::fixbug::LoginRequest* request,//回调 解析获得远程登录直接用反序列化的信息
               ::fixbug::LoginResponse* response, //回调 写回应远程信息的信息
               ::google::protobuf::Closure* done)//一个回调虚函数，通过其子类实现回传序列化回应信息
    {
        //框架给业务上报了请求参数LoginRequest,应用获取相应数据做本地业务
        std::string name=request->name();
        std::string pwd=request->pwd();

        //做本地业务
        bool login_result = Login(name,pwd);

        //把响应写入 包括错误码、错误消息、返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);//验证  0为正常，1为错误
        code->set_errmsg(""); //验证 Login do error!
        response->set_success(login_result);

        //执行回调操作 执行响应对象数据的序列化和网络发送（都是由框架来执行的）
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                  const ::fixbug::RegisterRequest* request,//框架接收到的反序列化数据
                  ::fixbug::RegisterResponse* response,
                  ::google::protobuf::Closure* done)
    {
        // 把request接收的到并反序列数据拿出
        uint32_t id=request->id();
        std::string name = request->name();
        std::string pwd=request->pwd();

        //调用本地业务开做
        bool ret=Register(id, name, pwd);

        // 准备要返回客户的信息
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        // 调用Run方法，将准备好的数据序列化并发回客户端
        done->Run();
    }
};

// 演示如何把一个本地服务变成一个分布式远程可调用服务

int main(int argc, char **argv)
{
    //以上类就是一个框架，如何使用这个框架呢？
    //以登录来模拟使用这个框架

    //定义一个框架初始化操作
    MprpcApplication::Init(argc,argv);// 该类里面包含一个configure类，解析配置文件+存储服务器名端口号

    //provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;//负责数据的序列化和反序列化，收发
    provider.NotifyService(new UserService());//把本服务机要提供的服务service子类传入解析存到 provider的服务，服务方法，描述map里

    //启动一个rpc服务发布节点 Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}
