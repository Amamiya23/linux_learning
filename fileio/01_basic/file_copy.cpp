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
    std::cerr << "错误: " << msg << " - " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
}

void copy_file(const char* src_path, const char* dst_path) {
    //只读格式打开O_RDONLY
    int src_fd = open(src_path, O_RDONLY);
    if (src_fd == -1) {
        error_exit("无法打开源文件" + std::string(src_path));
    }
    /*
    O_CREAT: 如果文件不存在则创建
    O_WRONLY: 以只写方式打开
    O_TRUNC: 如果文件存在且为O_WRONLY/O_RDWR，则截断为0
    0644: 文件权限 (rw-r--r--) */
    int dst_fd = open(dst_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (dst_fd == -1) {
        close(src_fd);
        error_exit("无法创建目标文件：" + std::string(dst_path));
    }

    //复制数据
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    size_t total_bytes = 0;
    //read()函数从文件描述符src_fd中读取数据到buffer中，最多读取BUFFER_SIZE字节，返回实际读取的字节数
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        //write()函数将buffer中的数据写入文件描述符dst_fd中，写入bytes_read字节，返回实际写入的字节数
        ssize_t bytes_written = write(dst_fd, buffer, bytes_read);
        if (bytes_written == -1) {
            close(src_fd);
            close(dst_fd);
            error_exit("写入文件失败");
        }
        total_bytes += bytes_written;
    }

    if (bytes_read == -1) {
        close(src_fd);
        close(dst_fd);
        error_exit("读取文件失败");
    }

    close(src_fd);
    close(dst_fd);

    std::cout << "文件复制成功！共 " << total_bytes << "字节" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "用法：" << argv[0] << " <源文件><目标文件>" << std::endl;
        return EXIT_FAILURE;
    }
    copy_file(argv[1], argv[2]);

    return EXIT_SUCCESS;
}
