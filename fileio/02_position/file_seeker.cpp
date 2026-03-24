/**
 * 文件定位器 - 阶段2：文件定位操作
 * 学习目标：掌握 lseek(), 文件指针操作
 * 
 * 编译: g++ -Wall -Wextra -o file_seeker 02_position/file_seeker.cpp
 * 运行: ./file_seeker test.txt
 */

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>

/**
 * 获取并显示文件大小
 */
off_t get_file_size(int fd) {
    struct stat st;
    if (fstat(fd, &st) == -1) {
        return -1;
    }
    return st.st_size;
}

/**
 * 演示文件定位操作
 */
void seek_demo(const char* filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        std::cerr << "无法打开文件: " << strerror(errno) << std::endl;
        return;
    }

    // 获取文件大小
    off_t file_size = get_file_size(fd);
    std::cout << "文件大小: " << file_size << " 字节" << std::endl;

    // 1. 从文件开头定位 (SEEK_SET)
    off_t pos = lseek(fd, 0, SEEK_SET);
    std::cout << "\n[SEEK_SET] 定位到文件开头, 位置: " << pos << std::endl;

    // 读取第一个字符
    char ch;
    if (read(fd, &ch, 1) > 0) {
        std::cout << "第一个字符: '" << ch << "'" << std::endl;
    }

    // 2. 从当前位置定位 (SEEK_CUR)
    pos = lseek(fd, 5, SEEK_CUR);
    std::cout << "\n[SEEK_CUR] 从当前位置后移5字节, 位置: " << pos << std::endl;

    // 读取5个字符
    char buffer[16];
    ssize_t n = read(fd, buffer, 5);
    if (n > 0) {
        buffer[n] = '\0';
        std::cout << "接下来5个字符: \"" << buffer << "\"" << std::endl;
    }

    // 3. 从文件末尾定位 (SEEK_END)
    pos = lseek(fd, -10, SEEK_END);
    std::cout << "\n[SEEK_END] 定位到文件末尾前10字节, 位置: " << pos << std::endl;

    n = read(fd, buffer, 10);
    if (n > 0) {
        buffer[n] = '\0';
        std::cout << "最后10个字符: \"" << buffer << "\"" << std::endl;
    }

    // 4. 获取当前文件位置
    pos = lseek(fd, 0, SEEK_CUR);
    std::cout << "\n当前文件位置: " << pos << std::endl;

    // 5. 使用 lseek 扩展文件大小 (创建空洞文件)
    std::cout << "\n===== 演示文件扩展 =====" << std::endl;
    close(fd);

    // 创建一个测试文件并扩展
    fd = open("sparse_file.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        write(fd, "START", 5);  // 写入开头5字节
        
        // 在中间创建空洞 - 从位置1000开始
        lseek(fd, 1000, SEEK_SET);
        write(fd, "MIDDLE", 6); // 写入到位置1006
        
        // 显示最终文件大小
        off_t new_size = lseek(fd, 0, SEEK_END);
        std::cout << "扩展后文件大小: " << new_size << " 字节" << std::endl;
        std::cout << "(实际占用磁盘空间可能更少，这是稀疏文件特性)" << std::endl;
        
        close(fd);
        std::cout << "已创建稀疏文件: sparse_file.txt" << std::endl;
    }

    close(fd);
}

/**
 * 演示随机读取 - 模拟读取文件特定位置
 */
void random_access_demo(const char* filepath) {
    std::cout << "\n===== 随机访问演示 =====" << std::endl;
    
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        std::cerr << "无法打开文件: " << strerror(errno) << std::endl;
        return;
    }

    // 定义要读取的位置列表
    struct {
        off_t offset;
        const char* desc;
    } positions[] = {
        {0, "文件开头"},
        {10, "位置10"},
        {50, "位置50"},
        {100, "位置100"},
    };

    char buf[16];
    for (const auto& p : positions) {
        // 跳转到指定位置
        off_t pos = lseek(fd, p.offset, SEEK_SET);
        if (pos == -1) {
            std::cout << "跳转到" << p.desc << "失败" << std::endl;
            continue;
        }
        
        // 读取数据
        ssize_t n = read(fd, buf, 10);
        if (n > 0) {
            buf[n] = '\0';
            std::cout << p.desc << " (偏移" << p.offset << "): \"" << buf << "\"" << std::endl;
        }
    }

    close(fd);
}

int main(int argc, char* argv[]) {
    const char* filepath = "test_file.txt";
    
    // 如果没有提供参数，创建测试文件
    if (access(filepath, F_OK) == -1) {
        std::cout << "创建测试文件..." << std::endl;
        int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd != -1) {
            const char* content = 
                "0123456789\n"
                "abcdefghijklmnopqrstuvwxyz\n"
                "这是一行中文内容\n"
                "Line 4: More content here\n"
                "Line 5: Even more content\n"
                "The last line of the file.\n";
            write(fd, content, strlen(content));
            close(fd);
        }
    }

    const char* target = (argc > 1) ? argv[1] : filepath;
    
    std::cout << "演示文件: " << target << std::endl;
    seek_demo(target);
    random_access_demo(target);

    return EXIT_SUCCESS;
}
