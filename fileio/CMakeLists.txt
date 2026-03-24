cmake_minimum_required(VERSION 3.10)
project(LinuxFileIO VERSION 1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -g)
endif()

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ============================================
# 阶段1: 基础文件操作
# ============================================
add_executable(file_copy 01_basic/file_copy.cpp)
add_executable(read_append 01_basic/read_append.cpp)

# ============================================
# 阶段2: 文件定位与状态
# ============================================
add_executable(file_seeker 02_position/file_seeker.cpp)
add_executable(file_stats 02_position/file_stats.cpp)

# ============================================
# 阶段3: 目录操作
# ============================================
add_executable(dir_list 03_directory/dir_list.cpp)
add_executable(dir_walker 03_directory/dir_walker.cpp)

# ============================================
# 阶段4: 高级文件操作
# ============================================
add_executable(file_lock 04_advanced/file_lock.cpp)
target_link_libraries(file_lock pthread)  # 需要pthread

add_executable(memory_map 04_advanced/memory_map.cpp)

# ============================================
# 阶段5: 实战项目 - 日志系统
# ============================================
add_executable(logger_demo 
    05_projects/logger/logger.h
    05_projects/logger/logger.cpp
    05_projects/logger/main.cpp
)

# ============================================
# 辅助函数和配置
# ============================================

# 创建输出目录的软链接或确保目录存在
add_custom_command(TARGET file_copy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
