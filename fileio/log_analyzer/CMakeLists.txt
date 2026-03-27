cmake_minimum_required(VERSION 3.10)
project(LogAnalyzer VERSION 1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -g)
endif()

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(include)

# ============================================
# 日志解析器库
# ============================================
add_library(logparser STATIC
    src/log_parser.cpp
    src/file_reader.cpp
    src/log_filter.cpp
    src/report_generator.cpp
    src/directory_walker.cpp
)

# 链接pthread（用于文件锁）
target_link_libraries(logparser pthread)

# ============================================
# 主程序：日志分析器
# ============================================
add_executable(log_analyzer src/main.cpp)
target_link_libraries(log_analyzer logparser)

# ============================================
# 实用工具程序
# ============================================
# 日志统计工具
add_executable(log_stats src/tools/log_stats.cpp)
target_link_libraries(log_stats logparser)

# 日志搜索工具
add_executable(log_search src/tools/log_search.cpp)
target_link_libraries(log_search logparser)

# 日志格式化工具
add_executable(log_format src/tools/log_format.cpp)
target_link_libraries(log_format logparser)

# ============================================
# 测试程序
# ============================================
add_executable(test_parser tests/test_parser.cpp)
target_link_libraries(test_parser logparser)

add_executable(test_file_reader tests/test_file_reader.cpp)
target_link_libraries(test_file_reader logparser)

# ============================================
# 创建输出目录
# ============================================
add_custom_command(TARGET log_analyzer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)