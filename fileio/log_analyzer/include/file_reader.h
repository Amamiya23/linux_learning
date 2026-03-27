#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <sys/stat.h>

namespace loganalyzer {

// 文件读取器基类
class FileReader {
public:
    virtual ~FileReader() = default;
    
    // 打开文件
    virtual bool open(const std::string& filepath) = 0;
    
    // 关闭文件
    virtual void close() = 0;
    
    // 读取所有行
    virtual std::vector<std::string> read_all_lines() = 0;
    
    // 逐行读取（回调方式）
    virtual void read_lines(std::function<bool(const std::string&, size_t)> callback) = 0;
    
    // 检查文件是否打开
    virtual bool is_open() const = 0;
    
    // 获取文件大小
    virtual size_t get_file_size() const = 0;
    
    // 获取文件路径
    std::string get_filepath() const { return filepath_; }
    
    // 检查文件是否存在
    static bool file_exists(const std::string& filepath);
    
    // 获取文件状态
    static bool get_file_stat(const std::string& filepath, struct stat& st);
    
protected:
    std::string filepath_;
};

// 标准IO文件读取器
class StandardFileReader : public FileReader {
public:
    StandardFileReader();
    ~StandardFileReader() override;
    
    bool open(const std::string& filepath) override;
    void close() override;
    std::vector<std::string> read_all_lines() override;
    void read_lines(std::function<bool(const std::string&, size_t)> callback) override;
    bool is_open() const override;
    size_t get_file_size() const override;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// 内存映射文件读取器
class MMapFileReader : public FileReader {
public:
    MMapFileReader();
    ~MMapFileReader() override;
    
    bool open(const std::string& filepath) override;
    void close() override;
    std::vector<std::string> read_all_lines() override;
    void read_lines(std::function<bool(const std::string&, size_t)> callback) override;
    bool is_open() const override;
    size_t get_file_size() const override;
    
    // 获取映射的内存指针
    const char* get_data() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// 文件读取器工厂
class FileReaderFactory {
public:
    enum class ReaderType {
        STANDARD,  // 标准IO
        MMAP       // 内存映射
    };
    
    static std::unique_ptr<FileReader> create_reader(ReaderType type);
    
    // 根据文件大小自动选择读取器
    static std::unique_ptr<FileReader> create_optimal_reader(const std::string& filepath);
};

} // namespace loganalyzer