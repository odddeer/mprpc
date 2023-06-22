cmake_minimum_required(VERSION 3.2.2)

# -o 起名
project(test)

#设置c++ -std
set(CMAKE_CXX_STANDARD 11)

# -g
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

link_directories(/usr/local/lib)

# cpp所有文件
aux_source_directory(. SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

## 链接 库 -l lib和尾部.a .so等不写
target_link_libraries(${PROJECT_NAME}  protobuf)