#include "test.pb.h"
#include<iostream>
#include <string>
using namespace fixbug;

int main()
{
//    LoginResponse rsp;
//    ResultCode* rc = rsp.mutable_result();
//    rc->set_errcode(1);
//    rc->set_errmsg("登录处理失败了");

    GetFriendListResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);//返回0，代表没错

    User *user1 = rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_name("li si");
    user2->set_age(22);
    user2->set_sex(User::MAN);

    std::cout<<rsp.friend_list_size()<<std::endl;

    return 0;
}

int main1()
{
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");

    std::string send_str;
    //序列化：
    if(req.SerializeToString(&send_str)){
        std::cout<<send_str.c_str()<<std::endl;
    }

    //反序列化
    LoginRequest reqB;
    if(reqB.ParseFromString(send_str)){
        std::cout<<reqB.name()<<std::endl;
        std::cout<<reqB.pwd()<<std::endl;
    }
    return 0;
}
