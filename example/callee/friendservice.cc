#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>

#include "logger.h"

class FriendService:public fixbug::FriendServiceRpc
{
public:
    // 本地业务编写
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::cout<<"do GetFriendsList service! userid:"<<userid<<std::endl;
        std::vector<std::string> vec;
        vec.push_back("gao yang");
        vec.push_back("liu hong");
        vec.push_back("wang shuo");
        return vec;
    }

    // 重写基类方法 框架解析要求，再执行命令
    void GetFriendsList(::google::protobuf::RpcController* controller,
                        const ::fixbug::GetFriendsListRequest* request,
                        ::fixbug::GetFriendsListResponse* response,
                        ::google::protobuf::Closure* done)
    {
        //从request解析需求
        uint32_t userid=request->userid();
        //调用本地业务执行
        std::vector<std::string> friendsList = GetFriendsList(userid);
        //在response里写执行返回值
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(std::string &name : friendsList)
        {
            std::string *p=response->add_friends();//这个add_friends会依次指向下一个数组
            *p=name;
        }
        done->Run();
    }
};

int main(int argc, char** argv)
{
    // 日志测试
    LOG_INFO("first log message!");
                         // 当前文件名  当前函数名   当前行数
    LOG_ERR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);

    //调用框架的初始化操作
    MprpcApplication::Init(argc,argv);

    // provider是一个rpc网络服务对象。把要提供的对象放入provider皆可
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    //启动一个rpc服务发布节点 Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();
    return 0;
}
