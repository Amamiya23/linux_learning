/**
 * 简单日志系统实现 - 阶段5：实战项目
 */

#include "logger.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <cstring>
#include <cerrno>
#include <algorithm>

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

Logger::~Logger() {
    shutdown();
}

bool Logger::init(const std::string& log_dir, const std::string& prefix,
                  size_t max_file_size, int max_files) {
    log_dir_ = log_dir;
    prefix_ = prefix;
    max_file_size_ = max_file_size;
    max_files_ = max_files;
    min_level_ = LogLevel::DEBUG;
    
    // 确保日志目录存在
    if (access(log_dir_.c_str(), F_OK) == -1) {
        if (mkdir(log_dir_.c_str(), 0755) == -1) {
            std::cerr << "无法创建日志目录: " << strerror(errno) << std::endl;
            return false;
        }
    }
    
    // 打开当前日志文件
    std::string logfile = get_current_logfile();
    current_fd_ = open(logfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (current_fd_ == -1) {
        std::cerr << "无法打开日志文件: " << strerror(errno) << std::endl;
        return false;
    }
    
    // 获取当前文件大小
    struct stat st;
    if (fstat(current_fd_, &st) == 0) {
        current_size_ = st.st_size;
    }
    
    info("日志系统初始化完成");
    return true;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < min_level_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string formatted = format_message(level, message);
    write_to_file(formatted);
}

void Logger::debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }
void Logger::info(const std::string& msg) { log(LogLevel::INFO, msg); }
void Logger::warning(const std::string& msg) { log(LogLevel::WARNING, msg); }
void Logger::error(const std::string& msg) { log(LogLevel::ERROR, msg); }
void Logger::fatal(const std::string& msg) { log(LogLevel::FATAL, msg); }

void Logger::set_level(LogLevel level) {
    min_level_ = level;
}

void Logger::shutdown() {
    if (current_fd_ != -1) {
        close(current_fd_);
        current_fd_ = -1;
    }
}

std::string Logger::get_current_logfile() {
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    
    char date[32];
    strftime(date, sizeof(date), "%Y%m%d", tm_info);
    
    return log_dir_ + "/" + prefix_ + "." + date + ".log";
}

void Logger::check_rotation() {
    // 检查是否需要按日期轮转
    // 这里简化处理：实际应根据日期变化重新打开文件
    
    // 检查文件大小是否超过限制
    if (current_size_ >= max_file_size_) {
        rotate_logs();
    }
}

void Logger::rotate_logs() {
    // 关闭当前文件
    if (current_fd_ != -1) {
        close(current_fd_);
        current_fd_ = -1;
    }
    
    // 重命名当前日志文件
    std::string current = get_current_logfile();
    char timestamp[32];
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", tm_info);
    
    std::string renamed = log_dir_ + "/" + prefix_ + "." + timestamp + ".log";
    rename(current.c_str(), renamed.c_str());
    
    // 删除旧文件
    DIR* dir = opendir(log_dir_.c_str());
    if (dir) {
        std::vector<std::string> log_files;
        struct dirent* entry;
        
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.find(prefix_) == 0 && name.find(".log") != std::string::npos) {
                log_files.push_back(log_dir_ + "/" + name);
            }
        }
        closedir(dir);
        
        // 按修改时间排序，删除旧文件
        if (static_cast<int>(log_files.size()) > max_files_) {
            // 简单删除策略：保留最新的max_files_个
            // 实际应按时间排序
        }
    }
    
    // 重新打开新文件
    current_fd_ = open(get_current_logfile().c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    current_size_ = 0;
}

void Logger::write_to_file(const std::string& message) {
    if (current_fd_ == -1) {
        return;
    }
    
    // 检查是否需要轮转
    check_rotation();
    
    // 使用文件锁确保多进程安全
    // 注意：这里简化处理，实际应在每次写入时加锁
    if (flock(current_fd_, LOCK_EX) == -1) {
        std::cerr << "加锁失败: " << strerror(errno) << std::endl;
        return;
    }
    
    ssize_t written = write(current_fd_, message.c_str(), message.length());
    
    if (written > 0) {
        current_size_ += written;
    }
    
    flock(current_fd_, LOCK_UN);
}

std::string Logger::format_message(LogLevel level, const std::string& message) {
    std::ostringstream oss;
    
    // 时间戳
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    oss << "[" << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S") << "] ";
    
    // 级别
    oss << "[" << level_to_string(level) << "] ";
    
    // 消息
    oss << message << "\n";
    
    return oss.str();
}

const char* Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO ";
        case LogLevel::WARNING: return "WARN ";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::FATAL:   return "FATAL";
        default:                return "UNKNOWN";
    }
}
