/**
 * 简单日志系统 - 阶段5：实战项目
 * 功能：按日期创建日志文件、日志轮转、多进程安全写入
 * 
 * 这是一个完整的日志系统框架，你可以基于此扩展
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <mutex>
#include <memory>

// 日志级别
enum class LogLevel {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Logger {
public:
    // 获取单例实例
    static Logger& instance();
    
    // 初始化日志系统
    bool init(const std::string& log_dir, 
              const std::string& prefix = "app",
              size_t max_file_size = 10 * 1024 * 1024,  // 10MB
              int max_files = 5);
    
    // 写入日志
    void log(LogLevel level, const std::string& message);
    
    // 便捷方法
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warning(const std::string& msg);
    void error(const std::string& msg);
    void fatal(const std::string& msg);
    
    // 设置日志级别
    void set_level(LogLevel level);
    
    // 关闭日志系统
    void shutdown();
    
private:
    Logger() = default;
    ~Logger();
    
    // 禁止拷贝
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // 获取当前日志文件路径
    std::string get_current_logfile();
    
    // 检查并执行日志轮转
    void check_rotation();
    
    // 执行日志轮转
    void rotate_logs();
    
    // 写入文件（线程安全）
    void write_to_file(const std::string& message);
    
    // 格式化日志消息
    std::string format_message(LogLevel level, const std::string& message);
    
    // 获取级别字符串
    const char* level_to_string(LogLevel level);
    
private:
    std::string log_dir_;           // 日志目录
    std::string prefix_;            // 日志文件前缀
    size_t max_file_size_;          // 单个文件最大大小
    int max_files_;                 // 保留文件数量
    LogLevel min_level_;            // 最小日志级别
    
    int current_fd_ = -1;           // 当前文件描述符
    size_t current_size_ = 0;      // 当前文件大小
    
    std::mutex mutex_;              // 互斥锁（多线程安全）
};

#endif // LOGGER_H
