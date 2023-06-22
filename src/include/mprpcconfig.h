#pragma once

#include <string>

#include <unordered_map>

// 框架读取配置文件类
class MprpcConfig
{//类模块逻辑：读取配置文件，进行解析（过滤注释、空格、空行等），把内容放在一个key-value的map里
    //并提供 给key 返value的 Load方法
public:
    //负责解析加载配置文件
    void LoadConfigFile(const char* config_file);
    //查询配置项信息 以key找对应value
    std::string Load(const std::string &key);
private:
    std::unordered_map<std::string,std::string> m_configMap;
    //去掉字符串前后的空格
    void Trim(std::string &src_buf);
};
