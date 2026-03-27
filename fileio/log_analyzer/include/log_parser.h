#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <optional>
#include <regex>

namespace loganalyzer {

// 日志级别枚举
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    UNKNOWN
};

// 日志条目结构
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string logger_name;
    std::string message;
    std::string raw_line;
    size_t line_number;
    
    // 转换为字符串
    std::string to_string() const;
    
    // 获取格式化的时间戳
    std::string get_timestamp_string() const;
};

// 日志解析器类
class LogParser {
public:
    LogParser();
    ~LogParser() = default;
    
    // 解析单行日志
    std::optional<LogEntry> parse_line(const std::string& line, size_t line_number);
    
    // 解析多行日志
    std::vector<LogEntry> parse_lines(const std::vector<std::string>& lines);
    
    // 设置日志格式模式
    void set_pattern(const std::string& pattern);
    
    // 获取当前模式
    std::string get_pattern() const;
    
    // 支持多种常见日志格式
    void use_default_pattern();
    void use_json_pattern();
    void use_csv_pattern();
    
private:
    std::regex pattern_regex_;
    std::string current_pattern_;
    
    // 解析日志级别字符串
    LogLevel parse_level(const std::string& level_str);
    
    // 解析时间戳
    std::chrono::system_clock::time_point parse_timestamp(const std::string& timestamp_str);
};

// 工具函数
std::string level_to_string(LogLevel level);
LogLevel string_to_level(const std::string& level_str);

} // namespace loganalyzer