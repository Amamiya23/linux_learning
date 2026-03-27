#include "../include/log_filter.h"
#include <algorithm>
#include <sstream>

namespace loganalyzer {

// FilterCriteria 实现
bool FilterCriteria::is_empty() const {
    return !min_level.has_value() &&
           !max_level.has_value() &&
           !logger_name.has_value() &&
           !message_contains.has_value() &&
           !start_time.has_value() &&
           !end_time.has_value() &&
           !message_pattern.has_value();
}

// LogFilter 实现
LogFilter::LogFilter() {}

void LogFilter::set_criteria(const FilterCriteria& criteria) {
    criteria_ = criteria;
}

const FilterCriteria& LogFilter::get_criteria() const {
    return criteria_;
}

bool LogFilter::matches(const LogEntry& entry) const {
    return check_level(entry) &&
           check_logger(entry) &&
           check_message(entry) &&
           check_time(entry);
}

std::vector<LogEntry> LogFilter::filter(const std::vector<LogEntry>& entries) const {
    std::vector<LogEntry> filtered;
    filtered.reserve(entries.size());
    
    for (const auto& entry : entries) {
        if (matches(entry)) {
            filtered.push_back(entry);
        }
    }
    
    return filtered;
}

void LogFilter::clear() {
    criteria_ = FilterCriteria{};
}

bool LogFilter::check_level(const LogEntry& entry) const {
    if (criteria_.min_level.has_value()) {
        if (entry.level < criteria_.min_level.value()) {
            return false;
        }
    }
    
    if (criteria_.max_level.has_value()) {
        if (entry.level > criteria_.max_level.value()) {
            return false;
        }
    }
    
    return true;
}

bool LogFilter::check_logger(const LogEntry& entry) const {
    if (!criteria_.logger_name.has_value()) {
        return true;
    }
    
    const std::string& target_logger = criteria_.logger_name.value();
    return entry.logger_name.find(target_logger) != std::string::npos;
}

bool LogFilter::check_message(const LogEntry& entry) const {
    // 检查简单文本包含
    if (criteria_.message_contains.has_value()) {
        const std::string& target = criteria_.message_contains.value();
        if (entry.message.find(target) == std::string::npos) {
            return false;
        }
    }
    
    // 检查正则表达式模式
    if (criteria_.message_pattern.has_value()) {
        if (!std::regex_search(entry.message, criteria_.message_pattern.value())) {
            return false;
        }
    }
    
    return true;
}

bool LogFilter::check_time(const LogEntry& entry) const {
    if (criteria_.start_time.has_value()) {
        if (entry.timestamp < criteria_.start_time.value()) {
            return false;
        }
    }
    
    if (criteria_.end_time.has_value()) {
        if (entry.timestamp > criteria_.end_time.value()) {
            return false;
        }
    }
    
    return true;
}

// LogStatistics 实现
LogStatistics::LogStatistics() : total_entries_(0) {}

void LogStatistics::collect(const std::vector<LogEntry>& entries) {
    for (const auto& entry : entries) {
        ++total_entries_;
        ++level_counts_[entry.level];
        update_time_range(entry);
        update_logger_counts(entry);
        update_error_counts(entry);
    }
}

void LogStatistics::clear() {
    total_entries_ = 0;
    level_counts_.clear();
    logger_counts_.clear();
    error_message_counts_.clear();
    earliest_time_.reset();
    latest_time_.reset();
}

size_t LogStatistics::get_total_entries() const {
    return total_entries_;
}

size_t LogStatistics::get_entries_by_level(LogLevel level) const {
    auto it = level_counts_.find(level);
    if (it != level_counts_.end()) {
        return it->second;
    }
    return 0;
}

std::map<LogLevel, size_t> LogStatistics::get_level_distribution() const {
    return level_counts_;
}

std::optional<std::chrono::system_clock::time_point> LogStatistics::get_earliest_time() const {
    return earliest_time_;
}

std::optional<std::chrono::system_clock::time_point> LogStatistics::get_latest_time() const {
    return latest_time_;
}

double LogStatistics::get_entries_per_second() const {
    if (!earliest_time_ || !latest_time_ || total_entries_ == 0) {
        return 0.0;
    }
    
    auto duration = latest_time_.value() - earliest_time_.value();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    
    if (seconds == 0) {
        return 0.0;
    }
    
    return static_cast<double>(total_entries_) / seconds;
}

std::vector<std::pair<std::string, size_t>> LogStatistics::get_top_loggers(size_t count) const {
    std::vector<std::pair<std::string, size_t>> result;
    result.reserve(logger_counts_.size());
    
    for (const auto& pair : logger_counts_) {
        result.emplace_back(pair.first, pair.second);
    }
    
    // 按计数降序排序
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });
    
    // 限制数量
    if (result.size() > count) {
        result.resize(count);
    }
    
    return result;
}

std::vector<std::pair<std::string, size_t>> LogStatistics::get_top_errors(size_t count) const {
    std::vector<std::pair<std::string, size_t>> result;
    result.reserve(error_message_counts_.size());
    
    for (const auto& pair : error_message_counts_) {
        result.emplace_back(pair.first, pair.second);
    }
    
    // 按计数降序排序
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });
    
    // 限制数量
    if (result.size() > count) {
        result.resize(count);
    }
    
    return result;
}

std::string LogStatistics::generate_report() const {
    std::ostringstream oss;
    
    oss << "=== Log Statistics Report ===\n";
    oss << "Total entries: " << total_entries_ << "\n\n";
    
    oss << "Level distribution:\n";
    for (const auto& pair : level_counts_) {
        oss << "  " << level_to_string(pair.first) << ": " << pair.second 
            << " (" << (100.0 * pair.second / total_entries_) << "%)\n";
    }
    
    oss << "\nTime range:\n";
    if (earliest_time_ && latest_time_) {
        auto earliest = std::chrono::system_clock::to_time_t(earliest_time_.value());
        auto latest = std::chrono::system_clock::to_time_t(latest_time_.value());
        oss << "  From: " << std::ctime(&earliest);
        oss << "  To:   " << std::ctime(&latest);
        oss << "  Entries per second: " << get_entries_per_second() << "\n";
    }
    
    oss << "\nTop loggers:\n";
    auto top_loggers = get_top_loggers(5);
    for (const auto& pair : top_loggers) {
        oss << "  " << pair.first << ": " << pair.second << "\n";
    }
    
    oss << "\nTop errors:\n";
    auto top_errors = get_top_errors(5);
    for (const auto& pair : top_errors) {
        oss << "  " << pair.first << ": " << pair.second << "\n";
    }
    
    return oss.str();
}

void LogStatistics::update_time_range(const LogEntry& entry) {
    if (!earliest_time_ || entry.timestamp < earliest_time_.value()) {
        earliest_time_ = entry.timestamp;
    }
    
    if (!latest_time_ || entry.timestamp > latest_time_.value()) {
        latest_time_ = entry.timestamp;
    }
}

void LogStatistics::update_logger_counts(const LogEntry& entry) {
    if (!entry.logger_name.empty()) {
        ++logger_counts_[entry.logger_name];
    }
}

void LogStatistics::update_error_counts(const LogEntry& entry) {
    if (entry.level >= LogLevel::ERROR) {
        ++error_message_counts_[entry.message];
    }
}

} // namespace loganalyzer