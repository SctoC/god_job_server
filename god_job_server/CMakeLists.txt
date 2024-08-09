# CMakeList.txt: god_job_server 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
# TODO: 如有需要，请添加测试并安装目标。
# CMakeList.txt: moduo_example 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required(VERSION 3.8)
project(MyMuduoServer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加库目录
link_directories(/home/ydy/good_job_server/muduo-2.0.2/muduo-master/build/release-install-cpp11/lib)

# 添加包含目录
# include_directories(/home/ydy/good_job_server/muduo-2.0.2/muduo-master/build/release-install-cpp11/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp)

# 添加源文件
set(JSONCPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/json_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/json_value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/json_writer.cpp
)

set(SOURCES
    god_job_server.cpp
    god_job_server.h
    MySQLClient.cpp
    # 其他源文件
)

# 创建可执行文件
add_executable(god_job_server ${SOURCES} ${JSONCPP_SOURCES})
# 将源代码添加到此项目的可执行文件。

# 链接库文件
target_link_libraries(god_job_server
    muduo_base
    muduo_http
    muduo_inspect
    muduo_net
    pthread
    mysqlclient
    # 如果需要，还可以链接其他库
)
