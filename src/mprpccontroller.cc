#include "mprpccontroller.h"

MprpcController::MprpcController()
{//构造函数，初始化没有错误，没有错误描述字符串
    m_failed= false;
    m_errText="";
}

void MprpcController::Reset()
{//重置错误标志bool，错误描述字符串
    m_failed= false;
    m_errText="";
}

bool MprpcController::Failed() const
{//返回错误码转态 true有错发送，否则为无false
    return m_failed;
}

std::string MprpcController::ErrorText() const
{//返回错误描述字符串
    return m_errText;
}

void MprpcController::SetFailed(const std::string &reason)
{//设置错误描述
    m_failed= true;
    m_errText=reason;
}

//目前未实现非主要功能
void MprpcController::StartCancel(){}
bool MprpcController::IsCanceled() const {return false;}
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback){}