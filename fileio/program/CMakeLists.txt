# 学生信息管理系统 CMake 构建配置
# 学习Linux文件I/O系统调用的示例项目

cmake_minimum_required(VERSION 3.10)

# 项目信息
project(StudentManagementSystem VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g")

# 头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 源文件
set(SOURCES
    src/main.cpp
    src/StudentManager.cpp
)

# 可执行文件
add_executable(student_sys ${SOURCES})

# 确保data目录存在
add_custom_command(TARGET student_sys POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/data
    COMMENT "创建data目录"
)

# 安装目标（可选）
install(TARGETS student_sys DESTINATION bin)

# 打印构建信息
message(STATUS "========== 构建配置 ==========")
message(STATUS "项目名称: ${PROJECT_NAME}")
message(STATUS "项目版本: ${PROJECT_VERSION}")
message(STATUS "C++ 标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "编译选项: ${CMAKE_CXX_FLAGS}")
message(STATUS "==============================")
