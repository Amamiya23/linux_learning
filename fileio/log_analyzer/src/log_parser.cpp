#include "../include/log_parser.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

namespace loganalyzer {

// LogEntry 实现
std::string LogEntry::to_string() const {
    std::ostringstream oss;
    oss << get_timestamp_string() << " [" << level_to_string(level) << "] ";
    if (!logger_name.empty()) {
        oss << logger_name << " - ";
    }
    oss << message;
    return oss.str();
}

std::string LogEntry::get_timestamp_string() const {
    auto time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm tm = *std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// LogParser 实现
LogParser::LogParser() {
    use_default_pattern();
}

std::optional<LogEntry> LogParser::parse_line(const std::string& line, size_t line_number) {
    if (line.empty()) {
        return std::nullopt;
    }
    
    std::smatch matches;
    if (!std::regex_search(line, matches, pattern_regex_)) {
        return std::nullopt;
    }
    
    LogEntry entry;
    entry.raw_line = line;
    entry.line_number = line_number;
    
    try {
        // 解析时间戳（假设第一个捕获组是时间戳）
        if (matches.size() > 1) {
            entry.timestamp = parse_timestamp(matches[1].str());
        }
        
        // 解析日志级别（假设第二个捕获组是级别）
        if (matches.size() > 2) {
            entry.level = parse_level(matches[2].str());
        }
        
        // 解析日志器名称（假设第三个捕获组是日志器名称）
        if (matches.size() > 3) {
            entry.logger_name = matches[3].str();
        }
        
        // 解析消息（假设第四个捕获组是消息）
        if (matches.size() > 4) {
            entry.message = matches[4].str();
        } else if (matches.size() > 3) {
            // 如果没有日志器名称，第三个捕获组可能是消息
            entry.message = matches[3].str();
        }
        
        return entry;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::vector<LogEntry> LogParser::parse_lines(const std::vector<std::string>& lines) {
    std::vector<LogEntry> entries;
    entries.reserve(lines.size());
    
    for (size_t i = 0; i < lines.size(); ++i) {
        auto entry = parse_line(lines[i], i + 1);
        if (entry) {
            entries.push_back(*entry);
        }
    }
    
    return entries;
}

void LogParser::set_pattern(const std::string& pattern) {
    try {
        pattern_regex_ = std::regex(pattern);
        current_pattern_ = pattern;
    } catch (const std::regex_error&) {
        // 如果正则表达式无效，使用默认模式
        use_default_pattern();
    }
}

std::string LogParser::get_pattern() const {
    return current_pattern_;
}

void LogParser::use_default_pattern() {
    // 默认模式：匹配常见日志格式
    // 例如：2023-01-01 12:00:00 [INFO] LoggerName - Message
    // 或者：2023-01-01 12:00:00 INFO Message
    set_pattern(R"((\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2})\s+\[?(\w+)\]?\s*(?:([^-\s]+)\s*-\s*)?(.*))");
}

void LogParser::use_json_pattern() {
    // JSON格式日志模式
    set_pattern(R"delim("timestamp"\s*:\s*"([^"]+)"[^}]*"level"\s*:\s*"(\w+)"[^}]*"logger"\s*:\s*"([^"]*)"[^}]*"message"\s*:\s*"([^"]*)")delim");
}

void LogParser::use_csv_pattern() {
    // CSV格式日志模式
    set_pattern(R"(([^,]+),(\w+),([^,]*),(.+))");
}

LogLevel LogParser::parse_level(const std::string& level_str) {
    std::string upper_level = level_str;
    std::transform(upper_level.begin(), upper_level.end(), upper_level.begin(), ::toupper);
    
    if (upper_level == "TRACE") return LogLevel::TRACE;
    if (upper_level == "DEBUG") return LogLevel::DEBUG;
    if (upper_level == "INFO") return LogLevel::INFO;
    if (upper_level == "WARN" || upper_level == "WARNING") return LogLevel::WARN;
    if (upper_level == "ERROR") return LogLevel::ERROR;
    if (upper_level == "FATAL") return LogLevel::FATAL;
    
    return LogLevel::UNKNOWN;
}

std::chrono::system_clock::time_point LogParser::parse_timestamp(const std::string& timestamp_str) {
    std::tm tm = {};
    std::istringstream ss(timestamp_str);
    
    // 尝试解析常见的时间戳格式
    // 格式1: 2023-01-01 12:00:00
    if (ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S")) {
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }
    
    // 格式2: 2023-01-01T12:00:00
    ss.clear();
    ss.str(timestamp_str);
    if (ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S")) {
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }
    
    // 如果无法解析，返回当前时间
    return std::chrono::system_clock::now();
}

// 工具函数实现
std::string level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

LogLevel string_to_level(const std::string& level_str) {
    std::string upper_level = level_str;
    std::transform(upper_level.begin(), upper_level.end(), upper_level.begin(), ::toupper);
    
    if (upper_level == "TRACE") return LogLevel::TRACE;
    if (upper_level == "DEBUG") return LogLevel::DEBUG;
    if (upper_level == "INFO") return LogLevel::INFO;
    if (upper_level == "WARN" || upper_level == "WARNING") return LogLevel::WARN;
    if (upper_level == "ERROR") return LogLevel::ERROR;
    if (upper_level == "FATAL") return LogLevel::FATAL;
    
    return LogLevel::UNKNOWN;
}

} // namespace loganalyzer