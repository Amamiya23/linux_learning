/**
 * 简单文件读取器 - 阶段1：基础文件操作
 * 学习目标：掌握文件打开模式、错误处理和读取逻辑
 * 
 * 编译: g++ -Wall -Wextra -o simple_reader 01_basic/simple_reader.cpp
 * 运行: ./simple_reader test.txt
 */

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

const size_t READ_BUF_SIZE = 256;

/**
 * 读取并显示文件内容
 * 展示不同的文件打开模式
 */
void read_file(const char* filepath) {
    // O_RDONLY: 只读模式
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        std::cerr << "无法打开文件 " << filepath << ": " 
                  << strerror(errno) << std::endl;
        return;
    }

    char buffer[READ_BUF_SIZE];
    ssize_t bytes_read;
    int line_count = 0;
    int total_bytes = 0;

    std::cout << "===== 文件内容 =====" << std::endl;
    
    // 逐块读取文件
    while ((bytes_read = read(fd, buffer, READ_BUF_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';  // 添加字符串结束符
        
        // 统计行数
        for (ssize_t i = 0; i < bytes_read; ++i) {
            if (buffer[i] == '\n') {
                line_count++;
            }
        }
        
        std::cout << buffer;
        total_bytes += bytes_read;
    }

    if (bytes_read == -1) {
        std::cerr << "读取错误: " << strerror(errno) << std::endl;
    }

    close(fd);

    std::cout << "\n===== 统计信息 =====" << std::endl;
    std::cout << "文件: " << filepath << std::endl;
    std::cout << "总字节数: " << total_bytes << std::endl;
    std::cout << "行数: " << line_count << std::endl;
}

/**
 * 演示 O_APPEND 模式 - 追加写入
 */
void append_demo(const char* filepath) {
    std::cout << "\n===== 追加写入演示 =====" << std::endl;
    
    // O_WRONLY | O_CREAT | O_APPEND: 追加模式
    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        std::cerr << "打开文件失败: " << strerror(errno) << std::endl;
        return;
    }

    const char* msg = "\n[追加] 这是追加的内容\n";
    ssize_t written = write(fd, msg, strlen(msg));
    
    if (written == -1) {
        std::cerr << "写入失败: " << strerror(errno) << std::endl;
    } else {
        std::cout << "成功追加 " << written << " 字节" << std::endl;
    }

    close(fd);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "用法: " << argv[0] << " <文件名> [追加内容]" << std::endl;
        
        // 创建一个测试文件用于演示
        std::cout << "\n创建测试文件..." << std::endl;
        int fd = open("test_file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd != -1) {
            const char* content = "第一行内容\n第二行内容\n第三行内容\n";
            write(fd, content, strlen(content));
            close(fd);
            std::cout << "测试文件 test_file.txt 已创建" << std::endl;
        }
        
        std::cout << "\n用法示例: " << argv[0] << " test_file.txt" << std::endl;
        return EXIT_FAILURE;
    }

    read_file(argv[1]);

    // 如果提供了第三个参数，演示追加功能
    if (argc >= 3) {
        append_demo(argv[1]);
        std::cout << "\n追加后重新读取文件:" << std::endl;
        read_file(argv[1]);
    }

    return EXIT_SUCCESS;
}
