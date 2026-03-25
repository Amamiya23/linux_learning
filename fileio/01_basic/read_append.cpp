/*
*使用系统调用实现文件复制
*/
#include <iostream>
#include <fcntl.h>      // open()
#include <unistd.h>     // read(), write(), close()
#include <cstring>     // strerror()
#include <cerrno>      // errno

//缓冲区大小
const size_t BUFFER_SIZE = 4096;

void error_exit(const std::string& msg) {
    std::cerr << msg << " - " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
}

void read_file(const char* file_path) {
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        error_exit("无法打开文件" + std::string(file_path));
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int line_count = 0;
    int total_bytes = 0;

    std::cout << "===== 文件内容 =====" << std::endl;
    //读取BUFFER_SIZE-1字节到buffer，留出一个字节给字符串结束符
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';  //在读取的数据后面添加\0，把缓冲区转换为C字符串

        //统计行数
        for (ssize_t i = 0;i < bytes_read;i++) {
            if (buffer[i] == '\n') {
                line_count++;
            }
        }
        //不停输出读取的缓冲区
        std::cout << buffer;
        total_bytes += bytes_read;
    }
    if (bytes_read == -1) {
        error_exit("读取错误");
    }

    close(fd);

    std::cout << "\n==== 统计信息 ===" << std::endl;
    std::cout << "文件:" << file_path << "  总字节数：" << total_bytes << "  行数：" << line_count << std::endl;
}

void append_file(const char* file_path, const char* content) {
    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        error_exit("打开文件失败");
    }
    //先添加一个换行
    write(fd, "\n", 1);
    ssize_t written = write(fd, content, strlen(content));
    if (written == -1) {
        error_exit("写入失败");
    }
    else {
        std::cout << "成功追加 " << written << "字节" << std::endl;
    }

    close(fd);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "用法：" << argv[0] << " <文件名> | " << argv[0] << " <文件名> [追加内容]" << std::endl;
        return EXIT_FAILURE;
    }
    if (argc >= 3) {
        append_file(argv[1], argv[2]);
        std::cout << "追加内容成功，请检查文件" << argv[1] << "的内容" << std::endl;
        read_file(argv[1]);
    }
    read_file(argv[1]);

    return EXIT_SUCCESS;
}
