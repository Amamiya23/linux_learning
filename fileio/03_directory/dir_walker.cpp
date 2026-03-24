/**
 * 目录遍历器 - 阶段3：递归目录操作
 * 学习目标：掌握递归遍历目录、文件过滤、符号链接处理
 * 
 * 编译: g++ -Wall -Wextra -o dir_walker 03_directory/dir_walker.cpp
 * 运行: ./dir_walker [起始目录]
 */

#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <algorithm>

// 全局配置
struct Config {
    bool show_hidden = false;     // 显示隐藏文件
    bool follow_symlinks = false;  // 跟踪符号链接
    int max_depth = 10;           // 最大递归深度
    std::string filter_ext;        // 文件扩展名过滤
    bool verbose = false;
};

Config g_config;

/**
 * 判断是否为隐藏文件
 */
bool is_hidden(const std::string& name) {
    return !name.empty() && name[0] == '.';
}

/**
 * 判断是否匹配扩展名
 */
bool matches_extension(const std::string& name) {
    if (g_config.filter_ext.empty()) {
        return true;
    }
    
    if (g_config.filter_ext[0] != '.') {
        g_config.filter_ext = "." + g_config.filter_ext;
    }
    
    return name.size() >= g_config.filter_ext.size() &&
           name.compare(name.size() - g_config.filter_ext.size(), 
                       g_config.filter_ext.size(), 
                       g_config.filter_ext) == 0;
}

/**
 * 文件信息结构体
 */
struct FileInfo {
    std::string path;
    std::string name;
    bool is_dir;
    bool is_link;
    off_t size;
    mode_t mode;
};

/**
 * 获取文件信息
 */
FileInfo get_file_info(const std::string& dirpath, const std::string& name) {
    FileInfo info;
    info.name = name;
    info.path = dirpath + "/" + name;
    
    struct stat st;
    
    // 根据配置决定是否跟踪符号链接
    int (*stat_func)(const char*, struct stat*) = 
        g_config.follow_symlinks ? stat : lstat;
    
    if (stat_func(info.path.c_str(), &st) == -1) {
        return info;
    }
    
    info.is_dir = S_ISDIR(st.st_mode);
    info.is_link = S_ISLNK(st.st_mode);
    info.size = st.st_size;
    info.mode = st.st_mode;
    
    return info;
}

/**
 * 递归遍历目录
 */
void walk_directory(const std::string& dirpath, int depth) {
    if (depth > g_config.max_depth) {
        std::cout << std::string(depth * 2, ' ') << "[最大深度限制]" << std::endl;
        return;
    }
    
    DIR* dir = opendir(dirpath.c_str());
    if (!dir) {
        std::cerr << "无法打开目录: " << dirpath << " - " << strerror(errno) << std::endl;
        return;
    }
    
    std::vector<FileInfo> entries;
    struct dirent* entry;
    
    // 读取所有目录项
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // 过滤隐藏文件
        if (!g_config.show_hidden && is_hidden(entry->d_name)) {
            continue;
        }
        
        // 获取文件信息
        FileInfo info = get_file_info(dirpath, entry->d_name);
        
        // 过滤扩展名
        if (!info.is_dir && !matches_extension(info.name)) {
            continue;
        }
        
        entries.push_back(info);
    }
    
    closedir(dir);
    
    // 排序：目录在前，文件在后，按名称排序
    std::sort(entries.begin(), entries.end(), [](const FileInfo& a, const FileInfo& b) {
        if (a.is_dir && !b.is_dir) return true;
        if (!a.is_dir && b.is_dir) return false;
        return a.name < b.name;
    });
    
    // 打印条目
    for (const auto& info : entries) {
        std::string indent(depth * 2, ' ');
        
        // 符号链接特殊标记
        std::string link_info;
        if (info.is_link) {
            char target[256];
            ssize_t len = readlink(info.path.c_str(), target, sizeof(target) - 1);
            if (len != -1) {
                target[len] = '\0';
                link_info = " -> " + std::string(target);
            }
        }
        
        if (info.is_dir) {
            std::cout << indent << "[DIR]  " << info.name << "/" << std::endl;
            
            // 递归进入子目录
            walk_directory(info.path, depth + 1);
        } else {
            std::string size_str = std::to_string(info.size);
            std::cout << indent << "[FILE] " << info.name 
                      << " (" << size_str << " bytes)" 
                      << link_info << std::endl;
        }
    }
}

/**
 * 统计目录信息
 */
void count_directory(const std::string& dirpath, int depth, 
                    int& file_count, int& dir_count, size_t& total_size) {
    if (depth > g_config.max_depth) {
        return;
    }
    
    DIR* dir = opendir(dirpath.c_str());
    if (!dir) {
        return;
    }
    
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        if (!g_config.show_hidden && is_hidden(entry->d_name)) {
            continue;
        }
        
        std::string fullpath = dirpath + "/" + entry->d_name;
        
        struct stat st;
        if (lstat(fullpath.c_str(), &st) == -1) {
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            dir_count++;
            count_directory(fullpath, depth + 1, file_count, dir_count, total_size);
        } else {
            file_count++;
            total_size += st.st_size;
        }
    }
    
    closedir(dir);
}

/**
 * 打印使用帮助
 */
void print_help(const char* prog) {
    std::cout << "用法: " << prog << " [选项] [目录]\n"
              << "\n选项:\n"
              << "  -a         显示隐藏文件\n"
              << "  -l         跟踪符号链接\n"
              << "  -d <n>     最大递归深度 (默认10)\n"
              << "  -e <ext>   按扩展名过滤 (如 .txt)\n"
              << "  -s         统计信息模式\n"
              << "  -h         显示帮助\n"
              << "\n示例:\n"
              << "  " << prog << " /home/user/documents\n"
              << "  " << prog << " -a -d 5 /var/log\n"
              << "  " << prog << " -e .cpp ~/project\n"
              << "  " << prog << " -s /home\n";
}

int main(int argc, char* argv[]) {
    std::string target_dir = ".";
    bool stats_mode = false;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        } else if (arg == "-a") {
            g_config.show_hidden = true;
        } else if (arg == "-l") {
            g_config.follow_symlinks = true;
        } else if (arg == "-d" && i + 1 < argc) {
            g_config.max_depth = std::stoi(argv[++i]);
        } else if (arg == "-e" && i + 1 < argc) {
            g_config.filter_ext = argv[++i];
        } else if (arg == "-s") {
            stats_mode = true;
        } else if (arg == "-v") {
            g_config.verbose = true;
        } else if (arg[0] != '-') {
            target_dir = arg;
        }
    }
    
    if (stats_mode) {
        // 统计模式
        int file_count = 0, dir_count = 0;
        size_t total_size = 0;
        
        std::cout << "统计目录: " << target_dir << std::endl;
        
        // 先统计当前目录
        DIR* dir = opendir(target_dir.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }
                
                std::string fullpath = target_dir + "/" + entry->d_name;
                struct stat st;
                
                if (g_config.follow_symlinks) {
                    stat(fullpath.c_str(), &st);
                } else {
                    lstat(fullpath.c_str(), &st);
                }
                
                if (S_ISDIR(st.st_mode)) {
                    count_directory(fullpath, 1, file_count, dir_count, total_size);
                } else {
                    file_count++;
                    total_size += st.st_size;
                }
            }
            closedir(dir);
        }
        
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "目录数量:  " << dir_count << std::endl;
        std::cout << "文件数量:  " << file_count << std::endl;
        std::cout << "总大小:    " << total_size << " 字节" << std::endl;
        std::cout << "总大小:    " << total_size / 1024.0 << " KB" << std::endl;
        std::cout << "总大小:    " << total_size / (1024.0 * 1024.0) << " MB" << std::endl;
        
    } else {
        // 树形显示模式
        std::cout << "目录树: " << target_dir << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        walk_directory(target_dir, 0);
    }
    
    return EXIT_SUCCESS;
}
