#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"


int main(int argc,char** argv)
{
    //整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);

    //演示调用远程发布的rpc方法 获取某人朋友信息
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());//stub消费端要传递Channel对象来调用
    //rpc服务所要提供的参数 输入
    fixbug::GetFriendsListRequest request;
    request.set_userid(1000);
    //rpc服务返回的结果接收
    fixbug::GetFriendsListResponse response;
    // 定义一个控制对象controller
    MprpcController controller;
    // 发起远程rpc方法的调用 阻塞等待远程rpc执行结果的返回
    stub.GetFriendsList(&controller,&request,&response, nullptr);//调用无论哪种方法都是 MprpcChannel::callmethod方法
    // 底层都是 RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络化

    //一次rpc调用完成，读调用的结构
    if(controller.Failed())//有错打印是哪个地方的错，没错执行成功提示
    {
        std::cout<<controller.ErrorText()<<std::endl;
    }
    else
    {
        if(0==response.result().errcode())
        {
            std::cout<<"rpc GetFriendsListResponse response success!"<<std::endl;
            int size=response.friends_size();
            for(int i=0;i<size;++i)
            {
                std::cout<<"index:"<<(i+1)<<" name:"<<response.friends(i)<<std::endl;
            }
        }
        else
        {
            std::cout<<"rpc GetFriendsListResponse response error: "<<response.result().errmsg()<<std::endl;
        }
    }


    return 0;
}