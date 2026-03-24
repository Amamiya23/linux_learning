/**
 * 目录列表器 - 阶段3：目录操作
 * 学习目标：掌握 opendir(), readdir(), closedir() 等目录操作函数
 * 
 * 编译: g++ -Wall -Wextra -o dir_list 03_directory/dir_list.cpp
 * 运行: ./dir_list [目录路径]
 */

#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <vector>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <fcntl.h>

/**
 * 获取文件类型字符串
 */
std::string get_type_str(unsigned char d_type) {
    switch (d_type) {
        case DT_BLK: return "b";
        case DT_CHR: return "c";
        case DT_DIR: return "d";
        case DT_FIFO: return "p";
        case DT_LNK: return "l";
        case DT_REG: return "-";
        case DT_SOCK: return "s";
        default: return "?";
    }
}

/**
 * 获取权限字符串
 */
std::string get_perm_str(mode_t mode) {
    std::string s;
    s += (mode & S_IRUSR) ? "r" : "-";
    s += (mode & S_IWUSR) ? "w" : "-";
    s += (mode & S_IXUSR) ? "x" : "-";
    s += (mode & S_IRGRP) ? "r" : "-";
    s += (mode & S_IWGRP) ? "w" : "-";
    s += (mode & S_IXGRP) ? "x" : "-";
    s += (mode & S_IROTH) ? "r" : "-";
    s += (mode & S_IWOTH) ? "w" : "-";
    s += (mode & S_IXOTH) ? "x" : "-";
    return s;
}

/**
 * 列出目录内容（基础版）
 */
void list_directory(const char* dirpath) {
    DIR* dir = opendir(dirpath);
    if (!dir) {
        std::cerr << "无法打开目录: " << dirpath << " - " << strerror(errno) << std::endl;
        return;
    }
    
    std::cout << "目录: " << dirpath << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    struct dirent* entry;
    int count = 0;
    
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        std::cout << entry->d_name << std::endl;
        count++;
    }
    
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "共 " << count << " 个条目" << std::endl;
    
    closedir(dir);
}

/**
 * 列出目录详细信息
 */
void list_directory_long(const char* dirpath) {
    DIR* dir = opendir(dirpath);
    if (!dir) {
        std::cerr << "无法打开目录: " << strerror(errno) << std::endl;
        return;
    }
    
    std::cout << "\n===== 详细列表 =====" << std::endl;
    std::cout << std::left << std::setw(12) << "权限" 
              << std::setw(8) << "链接"
              << std::setw(8) << "所有者"
              << std::setw(10) << "大小"
              << "  文件名" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    struct dirent* entry;
    struct stat st;
    int count = 0;
    size_t total_size = 0;
    
    // 构建完整路径
    std::vector<std::string> entries;
    
    while ((entry = readdir(dir)) != nullptr) {
        entries.push_back(entry->d_name);
    }
    
    closedir(dir);
    
    // 排序输出
    std::sort(entries.begin(), entries.end());
    
    for (const auto& name : entries) {
        // 跳过 . 和 ..
        if (name == "." || name == "..") {
            std::cout << std::left << std::setw(12) << "drwxr-xr-x"
                      << std::setw(8) << "2"
                      << std::setw(8) << "0"
                      << std::setw(10) << "4096"
                      << "  " << name << std::endl;
            continue;
        }
        
        std::string fullpath = std::string(dirpath) + "/" + name;
        
        if (stat(fullpath.c_str(), &st) == -1) {
            continue;
        }
        
        std::cout << std::left << std::setw(12) << get_perm_str(st.st_mode)
                  << std::setw(8) << st.st_nlink
                  << std::setw(8) << st.st_uid
                  << std::setw(10) << st.st_size;
        
        // 根据文件类型添加前缀
        char prefix = ' ';
        if (S_ISDIR(st.st_mode)) prefix = '/';
        else if (S_ISLNK(st.st_mode)) prefix = '@';
        else if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) prefix = '*';
        
        std::cout << "  " << prefix << name << std::endl;
        
        count++;
        total_size += st.st_size;
    }
    
    std::cout << std::string(60, '-') << std::endl;
    std::cout << "共 " << count << " 个文件, 总大小 " << total_size << " 字节" << std::endl;
}

/**
 * 演示目录创建和删除
 */
void dir_operations_demo() {
    const char* test_dir = "test_subdir";
    
    std::cout << "\n===== 目录操作演示 =====" << std::endl;
    
    // 创建目录
    if (mkdir(test_dir, 0755) == 0) {
        std::cout << "✓ 创建目录: " << test_dir << std::endl;
    } else {
        std::cerr << "✗ 创建目录失败: " << strerror(errno) << std::endl;
    }
    
    // 在目录中创建文件
    std::string test_file = std::string(test_dir) + "/file.txt";
    int fd = open(test_file.c_str(), O_WRONLY | O_CREAT, 0644);
    if (fd != -1) {
        write(fd, "test content", 12);
        close(fd);
        std::cout << "✓ 创建文件: " << test_file << std::endl;
    }
    
    // 再次列出目录
    std::cout << "\n创建后目录内容:" << std::endl;
    list_directory(test_dir);
    
    // 删除目录（需要先删除里面的文件）
    // 注意：rmdir只能删除空目录
    // 先删除文件
    unlink(test_file.c_str());
    std::cout << "✓ 删除文件: " << test_file << std::endl;
    
    // 再删除目录
    if (rmdir(test_dir) == 0) {
        std::cout << "✓ 删除目录: " << test_dir << std::endl;
    } else {
        std::cerr << "✗ 删除目录失败: " << strerror(errno) << std::endl;
    }
}

/**
 * 递归遍历目录
 */
void walk_directory_tree(const char* dirpath, int depth = 0) {
    DIR* dir = opendir(dirpath);
    if (!dir) {
        return;
    }
    
    std::string indent(depth * 2, ' ');
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        std::string fullpath = std::string(dirpath) + "/" + entry->d_name;
        
        // 使用d_type避免stat调用（如果文件系统支持）
        std::cout << indent;
        if (entry->d_type == DT_DIR) {
            std::cout << "[DIR]  " << entry->d_name << "/" << std::endl;
            walk_directory_tree(fullpath.c_str(), depth + 1);
        } else if (entry->d_type == DT_REG) {
            std::cout << "[FILE] " << entry->d_name << std::endl;
        } else if (entry->d_type == DT_LNK) {
            std::cout << "[LINK] " << entry->d_name << std::endl;
        } else {
            std::cout << "[OTHER] " << entry->d_name << std::endl;
        }
    }
    
    closedir(dir);
}

int main(int argc, char* argv[]) {
    const char* dirpath = (argc > 1) ? argv[1] : ".";
    
    // 基础列表
    list_directory(dirpath);
    
    // 详细列表（需要 stat）
    list_directory_long(dirpath);
    
    // 目录操作演示
    dir_operations_demo();
    
    // 递归遍历（如果有子目录）
    std::cout << "\n===== 目录树 =====" << std::endl;
    walk_directory_tree(dirpath);
    
    return EXIT_SUCCESS;
}
