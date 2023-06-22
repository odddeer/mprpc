#include "mprpcapplication.h"
#include <iostream>

#include <unistd.h>
#include <string>

MprpcConfig MprpcApplication::m_config;

void ShowArgsHelp()
{
    std::cout<<"format: command -i <configfile>"<<std::endl;
}

void MprpcApplication::Init(int argc,char **argv)
{
    if(argc<2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c=0;
    std::string config_file;
                         //"i"表示没有参数要填 "i:"表示要填参数
    while((c= getopt(argc,argv,"i:"))!=-1)
    {
        switch(c)
        {
            case 'i': //参数正确，存入config_file中
                 config_file = optarg;//:后面的数会存optarg里
                 break;
            case '?': //参数无效
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            case ':': // 没写多余参数
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }
    // 开始加载配置文件了 rpcserver_ip rpcserver_port zookeeper_ip zookeeper_port
    m_config.LoadConfigFile(config_file.c_str());

//    std::cout<<"rpcserverip:"<<m_config.Load("rpcserverip")<<std::endl;
//    std::cout<<"rpcserverport:"<<m_config.Load("rpcserverport")<<std::endl;
//    std::cout<<"zookeeperip:"<<m_config.Load("zookeeperip")<<std::endl;
//    std::cout<<"zookeeperport:"<<m_config.Load("zookeeperport")<<std::endl;
}

MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}