/**
 * 文件状态查看器 - 阶段2：文件状态操作
 * 学习目标：掌握 fstat(), stat(), access() 等函数
 * 
 * 编译: g++ -Wall -Wextra -o file_stats 02_position/file_stats.cpp
 * 运行: ./file_stats filename
 */

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <iomanip>

/**
 * 辅助函数：格式化文件类型
 */
std::string get_file_type(mode_t mode) {
    if (S_ISREG(mode)) return "普通文件";
    if (S_ISDIR(mode)) return "目录";
    if (S_ISLNK(mode)) return "符号链接";
    if (S_ISBLK(mode)) return "块设备";
    if (S_ISCHR(mode)) return "字符设备";
    if (S_ISFIFO(mode)) return "管道/FIFO";
    if (S_ISSOCK(mode)) return "套接字";
    return "未知类型";
}

/**
 * 辅助函数：格式化权限位
 */
std::string get_permissions(mode_t mode) {
    std::string perms;
    
    // 所有者权限
    perms += (mode & S_IRUSR) ? "r" : "-";
    perms += (mode & S_IWUSR) ? "w" : "-";
    perms += (mode & S_IXUSR) ? "x" : "-";
    
    // 组权限
    perms += (mode & S_IRGRP) ? "r" : "-";
    perms += (mode & S_IWGRP) ? "w" : "-";
    perms += (mode & S_IXGRP) ? "x" : "-";
    
    // 其他用户权限
    perms += (mode & S_IROTH) ? "r" : "-";
    perms += (mode & S_IWOTH) ? "w" : "-";
    perms += (mode & S_IXOTH) ? "x" : "-";
    
    return perms;
}

/**
 * 使用 stat() 获取文件状态
 */
void show_file_stats(const char* filepath) {
    struct stat st;
    
    // stat() - 通过路径获取文件状态（会跟踪符号链接）
    if (stat(filepath, &st) == -1) {
        std::cerr << "无法获取文件状态: " << strerror(errno) << std::endl;
        return;
    }
    
    std::cout << "===== 文件详细信息 =====" << std::endl;
    std::cout << "文件路径: " << filepath << std::endl;
    std::cout << "文件类型: " << get_file_type(st.st_mode) << std::endl;
    std::cout << "权限位:   " << get_permissions(st.st_mode) << std::endl;
    std::cout << "inode号:  " << st.st_ino << std::endl;
    std::cout << "设备号:   " << st.st_dev << std::endl;
    std::cout << "硬链接数: " << st.st_nlink << std::endl;
    std::cout << "UID:      " << st.st_uid << std::endl;
    std::cout << "GID:      " << st.st_gid << std::endl;
    
    // 文件大小
    std::cout << "文件大小: " << st.st_size << " 字节";
    if (st.st_size > 1024) {
        std::cout << " (" << std::fixed << std::setprecision(2) 
                  << st.st_size / 1024.0 << " KB)";
    }
    if (st.st_size > 1024 * 1024) {
        std::cout << " (" << st.st_size / (1024.0 * 1024.0) << " MB)";
    }
    std::cout << std::endl;
    
    // 块大小和占用块数
    std::cout << "块大小:   " << st.st_blksize << " 字节" << std::endl;
    std::cout << "占用块数: " << st.st_blocks << " (每块512字节)" << std::endl;
    std::cout << "实际占用: " << st.st_blocks * 512 << " 字节" << std::endl;
    
    // 时间戳
    std::cout << "\n===== 时间信息 =====" << std::endl;
    std::cout << "访问时间 (atime): " << std::ctime(&st.st_atim.tv_sec);
    std::cout << "修改时间 (mtime): " << std::ctime(&st.st_mtim.tv_sec);
    std::cout << "改变时间 (ctime): " << std::ctime(&st.st_ctim.tv_sec);
}

/**
 * 使用 lstat() - 不跟踪符号链接
 */
void show_lstat_info(const char* filepath) {
    struct stat st;
    
    if (lstat(filepath, &st) == -1) {
        return;
    }
    
    if (S_ISLNK(st.st_mode)) {
        std::cout << "\n===== 符号链接信息 =====" << std::endl;
        
        char link_target[256];
        ssize_t len = readlink(filepath, link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            std::cout << "链接指向: " << link_target << std::endl;
        }
    }
}

/**
 * 使用 access() - 检查文件权限
 */
void check_access(const char* filepath) {
    std::cout << "\n===== 权限检查 (access()) =====" << std::endl;
    
    struct {
        int mode;
        const char* name;
    } checks[] = {
        {F_OK, "文件是否存在"},
        {R_OK, "可读"},
        {W_OK, "可写"},
        {X_OK, "可执行"},
    };
    
    for (const auto& check : checks) {
        if (access(filepath, check.mode) == 0) {
            std::cout << "✓ " << check.name << std::endl;
        } else {
            std::cout << "✗ " << check.name << " (" << strerror(errno) << ")" << std::endl;
        }
    }
}

/**
 * 使用 fstat() - 通过文件描述符获取状态
 */
void show_fd_stats(int fd) {
    struct stat st;
    if (fstat(fd, &st) == -1) {
        std::cerr << "fstat失败: " << strerror(errno) << std::endl;
        return;
    }
    
    std::cout << "\n===== fstat() (通过文件描述符) =====" << std::endl;
    std::cout << "文件大小: " << st.st_size << " 字节" << std::endl;
    std::cout << "权限位:   " << get_permissions(st.st_mode) << std::endl;
}

/**
 * 演示文件时间戳修改
 */
void touch_demo() {
    std::cout << "\n===== 模拟 touch 命令 =====" << std::endl;
    
    const char* test_file = "touch_test.txt";
    
    // 创建空文件
    int fd = open(test_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        close(fd);
        std::cout << "创建文件: " << test_file << std::endl;
    }
    
    // 使用 utimensat() 或 futimens() 可以修改时间戳
    // 这里仅作演示，实际使用需要包含 <fcntl.h>
    struct timespec times[2] = {
        {0, UTIME_NOW},   // 访问时间
        {0, UTIME_NOW},   // 修改时间
    };
    
    if (utimensat(0, test_file, times, 0) == 0) {
        std::cout << "更新文件时间戳成功" << std::endl;
    }
    
    show_file_stats(test_file);
}

int main(int argc, char* argv[]) {
    const char* filepath = "test_file.txt";
    const char* target = (argc > 1) ? argv[1] : filepath;
    
    std::cout << "查看文件: " << target << std::endl;
    show_file_stats(target);
    show_lstat_info(target);
    check_access(target);
    
    // 通过文件描述符演示
    int fd = open(target, O_RDONLY);
    if (fd != -1) {
        show_fd_stats(fd);
        close(fd);
    }
    
    // touch演示
    touch_demo();
    
    return EXIT_SUCCESS;
}
