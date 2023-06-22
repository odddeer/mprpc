aux_source_directory(. SRC_LIST)

#add_library(mprpc SHARED ${SRC_LIST})
add_library(mprpc ${SRC_LIST}) #也改成和muduo一致的静态库 ，不然报错
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)