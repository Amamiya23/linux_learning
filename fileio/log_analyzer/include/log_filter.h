#pragma once

#include "log_parser.h"
#include <functional>
#include <map>
#include <chrono>

namespace loganalyzer {

// 过滤条件
struct FilterCriteria {
    std::optional<LogLevel> min_level;
    std::optional<LogLevel> max_level;
    std::optional<std::string> logger_name;
    std::optional<std::string> message_contains;
    std::optional<std::chrono::system_clock::time_point> start_time;
    std::optional<std::chrono::system_clock::time_point> end_time;
    std::optional<std::regex> message_pattern;
    
    // 检查是否为空过滤器
    bool is_empty() const;
};

// 日志过滤器类
class LogFilter {
public:
    LogFilter();
    ~LogFilter() = default;
    
    // 设置过滤条件
    void set_criteria(const FilterCriteria& criteria);
    
    // 获取当前过滤条件
    const FilterCriteria& get_criteria() const;
    
    // 过滤单个日志条目
    bool matches(const LogEntry& entry) const;
    
    // 过滤日志条目列表
    std::vector<LogEntry> filter(const std::vector<LogEntry>& entries) const;
    
    // 清除过滤条件
    void clear();
    
private:
    FilterCriteria criteria_;
    
    // 检查各个条件
    bool check_level(const LogEntry& entry) const;
    bool check_logger(const LogEntry& entry) const;
    bool check_message(const LogEntry& entry) const;
    bool check_time(const LogEntry& entry) const;
};

// 日志统计类
class LogStatistics {
public:
    LogStatistics();
    ~LogStatistics() = default;
    
    // 收集统计信息
    void collect(const std::vector<LogEntry>& entries);
    
    // 清除统计信息
    void clear();
    
    // 获取统计结果
    size_t get_total_entries() const;
    size_t get_entries_by_level(LogLevel level) const;
    std::map<LogLevel, size_t> get_level_distribution() const;
    
    // 获取时间范围
    std::optional<std::chrono::system_clock::time_point> get_earliest_time() const;
    std::optional<std::chrono::system_clock::time_point> get_latest_time() const;
    
    // 获取日志频率（条/秒）
    double get_entries_per_second() const;
    
    // 获取最常见的日志器名称
    std::vector<std::pair<std::string, size_t>> get_top_loggers(size_t count = 10) const;
    
    // 获取最常见的错误消息
    std::vector<std::pair<std::string, size_t>> get_top_errors(size_t count = 10) const;
    
    // 生成统计报告字符串
    std::string generate_report() const;
    
private:
    size_t total_entries_;
    std::map<LogLevel, size_t> level_counts_;
    std::map<std::string, size_t> logger_counts_;
    std::map<std::string, size_t> error_message_counts_;
    std::optional<std::chrono::system_clock::time_point> earliest_time_;
    std::optional<std::chrono::system_clock::time_point> latest_time_;
    
    // 辅助函数
    void update_time_range(const LogEntry& entry);
    void update_logger_counts(const LogEntry& entry);
    void update_error_counts(const LogEntry& entry);
};

} // namespace loganalyzer