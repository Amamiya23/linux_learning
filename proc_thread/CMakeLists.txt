cmake_minimum_required(VERSION 3.10)

project(ProcessThreadLearning VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g")

# 设置可执行文件输出目录为项目根目录下的 bin 文件夹
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(fork_demo 01_process/fork_demo.cpp)
add_executable(wait_demo 02_process_wait/wait_demo.cpp)
add_executable(pipe_demo 03_ipc_pipe/pipe_demo.cpp)
add_executable(thread_demo 04_threads/thread_demo.cpp)
add_executable(counter_demo 05_threads_mutex/counter_demo.cpp)
add_executable(producer_consumer 06_threads_condition/producer_consumer.cpp)

find_package(Threads REQUIRED)
target_link_libraries(thread_demo PRIVATE Threads::Threads)
target_link_libraries(counter_demo PRIVATE Threads::Threads)
target_link_libraries(producer_consumer PRIVATE Threads::Threads)

message(STATUS "========== 构建配置 ==========")
message(STATUS "项目名称: ${PROJECT_NAME}")
message(STATUS "项目版本: ${PROJECT_VERSION}")
message(STATUS "C++ 标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "编译选项: ${CMAKE_CXX_FLAGS}")
message(STATUS "==============================")
