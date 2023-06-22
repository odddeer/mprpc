#pragma once
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

// mprpc框架的基础类 负责框架的初始化操作
class MprpcApplication
{
public:
    static void Init(int argc,char **argv);//初始化：用m_config对象对配置文件进行解析
    static MprpcApplication& GetInstance();//获取这个单例mprpc框架
    static MprpcConfig& GetConfig();// 获取配置信息对象

private:
    static MprpcConfig m_config;//静态变量记得初始化

    MprpcApplication(){};
    MprpcApplication(const MprpcApplication&)=delete;
    MprpcApplication(MprpcApplication&&)=delete;
};
