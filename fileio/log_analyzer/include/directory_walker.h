#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <sys/types.h>
#include <dirent.h>

namespace loganalyzer {

// 文件信息结构
struct FileInfo {
    std::string filepath;
    std::string filename;
    std::string extension;
    size_t file_size;
    time_t modification_time;
    bool is_regular_file;
    bool is_directory;
};

// 目录遍历器类
class DirectoryWalker {
public:
    DirectoryWalker();
    ~DirectoryWalker();
    
    // 设置要遍历的目录
    bool set_directory(const std::string& dirpath);
    
    // 设置文件过滤器
    void set_file_filter(std::function<bool(const FileInfo&)> filter);
    
    // 设置是否递归遍历子目录
    void set_recursive(bool recursive);
    
    // 设置文件扩展名过滤器
    void set_extension_filter(const std::vector<std::string>& extensions);
    
    // 遍历目录，返回匹配的文件列表
    std::vector<FileInfo> walk();
    
    // 遍历目录，使用回调函数处理每个文件
    void walk(std::function<bool(const FileInfo&)> callback);
    
    // 获取目录中的所有日志文件
    std::vector<FileInfo> get_log_files(const std::string& dirpath);
    
    // 检查路径是否为目录
    static bool is_directory(const std::string& path);
    
    // 检查路径是否为普通文件
    static bool is_regular_file(const std::string& path);
    
    // 获取文件扩展名
    static std::string get_extension(const std::string& filepath);
    
    // 获取文件名（不含路径）
    static std::string get_filename(const std::string& filepath);
    
    // 获取目录路径
    static std::string get_directory(const std::string& filepath);
    
    // 连接路径
    static std::string join_path(const std::string& dir, const std::string& file);
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    
    // 遍历单个目录
    void walk_directory(const std::string& dirpath, std::vector<FileInfo>& results);
    
    // 获取文件信息
    FileInfo get_file_info(const std::string& filepath);
};

// 多文件处理器类
class MultiFileProcessor {
public:
    MultiFileProcessor();
    ~MultiFileProcessor() = default;
    
    // 添加要处理的文件
    void add_file(const std::string& filepath);
    
    // 添加目录中的所有日志文件
    void add_directory(const std::string& dirpath, bool recursive = true);
    
    // 设置文件处理回调
    void set_file_handler(std::function<bool(const std::string&)> handler);
    
    // 处理所有文件
    void process_all();
    
    // 获取文件总数
    size_t get_file_count() const;
    
    // 获取已处理文件数
    size_t get_processed_count() const;
    
    // 清除文件列表
    void clear();
    
private:
    std::vector<std::string> files_;
    std::function<bool(const std::string&)> handler_;
    size_t processed_count_;
};

} // namespace loganalyzer