#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <iomanip>

void error_exit(const std::string& msg) {
    std::cerr << msg << " - " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
}

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

void show_file_status(const char* file_path) {
    struct stat st;
    if (stat(file_path, &st) == -1) {
        error_exit("无法获取文件状态");
        return;
    }

    std::cout << "===== 文件详细信息 =====" << std::endl;
    std::cout << "文件路径: " << file_path << std::endl;
    std::cout << "文件类型: " << get_file_type(st.st_mode) << std::endl;
    std::cout << "权限位:   " << get_permissions(st.st_mode) << std::endl;
    std::cout << "inode号:  " << st.st_ino << std::endl;
    std::cout << "设备号:   " << st.st_dev << std::endl;
    std::cout << "硬链接数: " << st.st_nlink << std::endl;
    std::cout << "UID:       " << st.st_uid << std::endl;
    std::cout << "GID:       " << st.st_gid << std::endl;
    std::cout << "文件大小：";
    if (st.st_size < 1024) {
        std::cout << st.st_size << "B";
    }
    else if (1024 <= st.st_size < 1024 * 1024) {
        std::cout << st.st_size / 1024.0 << "KB";
    }
    else {
        std::cout << st.st_size / (1024.0 * 1024.0) << "MB";
    }

    std::cout << "\n===== 时间信息 =====" << std::endl;
    std::cout << "访问时间 (atime): " << std::ctime(&st.st_atim.tv_sec);
    std::cout << "修改时间 (mtime): " << std::ctime(&st.st_mtim.tv_sec);
    std::cout << "改变时间 (ctime): " << std::ctime(&st.st_ctim.tv_sec);
}

//当传入是符号链接时
void show_lstat_info(const char* file_path) {
    struct stat st;

    if (lstat(file_path, &st) == -1) {
        std::cout << "无法获取文件状态: " << strerror(errno) << std::endl;
        return;
    }

    if (S_ISLNK(st.st_mode)) {
        std::cout << "=====符号链接信息=====" << std::endl;
        char link_target[256];
        ssize_t len = readlink(file_path, link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            std::cout << "链接指向：" << link_target << std::endl;
        }
    }
}

//使用access()检查文件权限
void check_access(const char* file_path) {
    std::cout << "=====权限检查=====" << std::endl;

    struct Check {
        int mode;
        const char* name;
    };
    Check checks[] = {
        {F_OK, "文件是否存在"},
        {R_OK, "可读"},
        {W_OK, "可写"},
        {X_OK, "可执行"},
    };
    for (const auto& check : checks) {
        if (access(file_path, check.mode) == 0) {
            std::cout <<"yes"<<check.name << std::endl;
        }
        else {
            std::cout << "no"<<check.name <<std::endl;
        }
    }
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "用法：" << argv[0] << " <文件名>" << std::endl;
        return EXIT_FAILURE;
    }

    show_file_status(argv[1]);
    show_lstat_info(argv[1]);
    check_access(argv[1]);
    return 0;
}