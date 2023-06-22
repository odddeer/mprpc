# cmake最低要求
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

# -g
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 项目名称
project(mprpc)

# 设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置项目库文件输出的路径 生成库自己用
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目编译头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

# 设置项目库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

# src包含了mprpc框架所有的相关代码
add_subdirectory(src) # 放框架代码
# example包含了mprpc框架使用的示例代码
add_subdirectory(example) # 放使用框架的消费者，服务者代码