#pragma once
#include <google/protobuf/service.h>
#include <string>

class MprpcController:public google::protobuf::RpcController
{//该类用于反映mprpcchannel的回应客户端申请、回应解析等过程中发生的错误问题
public:
    MprpcController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);

    //目前未实现具体的功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);
private:
    bool m_failed; // RPC方法执行过程中的转态
    std::string m_errText; // RPC方法执行过程中的错误信息
};
