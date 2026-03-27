#include "../include/report_generator.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace loganalyzer {

// ReportGenerator 实现
class ReportGenerator::Impl {
public:
    Impl() : format_(ReportFormat::TEXT), is_open_(false) {}
    
    bool set_output_file(const std::string& filepath) {
        file_.open(filepath, std::ios::out | std::ios::trunc);
        if (!file_.is_open()) {
            return false;
        }
        filepath_ = filepath;
        is_open_ = true;
        return true;
    }
    
    void set_format(ReportFormat format) {
        format_ = format;
    }
    
    void set_title(const std::string& title) {
        title_ = title;
    }
    
    bool generate_entries_report(const std::vector<LogEntry>& entries) {
        if (!is_open_) {
            return false;
        }
        
        write_header();
        
        for (const auto& entry : entries) {
            std::string line;
            switch (format_) {
                case ReportFormat::TEXT:
                    line = format_entry_text(entry);
                    break;
                case ReportFormat::CSV:
                    line = format_entry_csv(entry);
                    break;
                case ReportFormat::JSON:
                    line = format_entry_json(entry);
                    break;
                case ReportFormat::HTML:
                    line = format_entry_html(entry);
                    break;
            }
            file_ << line << "\n";
        }
        
        write_footer();
        return true;
    }
    
    bool generate_statistics_report(const LogStatistics& statistics) {
        if (!is_open_) {
            return false;
        }
        
        write_header();
        
        if (format_ == ReportFormat::TEXT) {
            file_ << statistics.generate_report();
        } else if (format_ == ReportFormat::JSON) {
            file_ << "{\n";
            file_ << "  \"total_entries\": " << statistics.get_total_entries() << ",\n";
            file_ << "  \"level_distribution\": {\n";
            auto distribution = statistics.get_level_distribution();
            for (auto it = distribution.begin(); it != distribution.end(); ++it) {
                file_ << "    \"" << level_to_string(it->first) << "\": " << it->second;
                if (std::next(it) != distribution.end()) {
                    file_ << ",";
                }
                file_ << "\n";
            }
            file_ << "  }\n";
            file_ << "}\n";
        }
        
        write_footer();
        return true;
    }
    
    bool generate_comprehensive_report(const std::vector<LogEntry>& entries, 
                                     const LogStatistics& statistics) {
        if (!is_open_) {
            return false;
        }
        
        write_header();
        
        // 先写入统计信息
        if (format_ == ReportFormat::TEXT) {
            file_ << statistics.generate_report() << "\n\n";
            file_ << "=== Log Entries ===\n";
        }
        
        // 写入日志条目
        for (const auto& entry : entries) {
            std::string line;
            switch (format_) {
                case ReportFormat::TEXT:
                    line = format_entry_text(entry);
                    break;
                case ReportFormat::CSV:
                    line = format_entry_csv(entry);
                    break;
                case ReportFormat::JSON:
                    line = format_entry_json(entry);
                    break;
                case ReportFormat::HTML:
                    line = format_entry_html(entry);
                    break;
            }
            file_ << line << "\n";
        }
        
        write_footer();
        return true;
    }
    
    bool generate_filtered_report(const std::vector<LogEntry>& entries,
                                const LogFilter& filter) {
        if (!is_open_) {
            return false;
        }
        
        auto filtered_entries = filter.filter(entries);
        return generate_entries_report(filtered_entries);
    }
    
    void close() {
        if (file_.is_open()) {
            file_.close();
        }
        is_open_ = false;
    }
    
    bool is_open() const {
        return is_open_;
    }
    
private:
    std::ofstream file_;
    std::string filepath_;
    ReportFormat format_;
    std::string title_;
    bool is_open_;
    
    std::string format_entry_text(const LogEntry& entry) {
        return entry.to_string();
    }
    
    std::string format_entry_csv(const LogEntry& entry) {
        std::ostringstream oss;
        oss << entry.get_timestamp_string() << ","
            << level_to_string(entry.level) << ","
            << entry.logger_name << ","
            << "\"" << entry.message << "\"";
        return oss.str();
    }
    
    std::string format_entry_json(const LogEntry& entry) {
        std::ostringstream oss;
        oss << "{";
        oss << "\"timestamp\":\"" << entry.get_timestamp_string() << "\",";
        oss << "\"level\":\"" << level_to_string(entry.level) << "\",";
        oss << "\"logger\":\"" << entry.logger_name << "\",";
        oss << "\"message\":\"" << entry.message << "\"";
        oss << "}";
        return oss.str();
    }
    
    std::string format_entry_html(const LogEntry& entry) {
        std::ostringstream oss;
        oss << "<tr>";
        oss << "<td>" << entry.get_timestamp_string() << "</td>";
        oss << "<td>" << level_to_string(entry.level) << "</td>";
        oss << "<td>" << entry.logger_name << "</td>";
        oss << "<td>" << entry.message << "</td>";
        oss << "</tr>";
        return oss.str();
    }
    
    void write_header() {
        if (!title_.empty()) {
            file_ << title_ << "\n";
        }
        
        switch (format_) {
            case ReportFormat::TEXT:
                file_ << "Generated at: " << get_current_time_string() << "\n\n";
                break;
            case ReportFormat::CSV:
                file_ << "Timestamp,Level,Logger,Message\n";
                break;
            case ReportFormat::JSON:
                file_ << "{\n  \"entries\": [\n";
                break;
            case ReportFormat::HTML:
                file_ << "<!DOCTYPE html>\n<html>\n<head>\n";
                file_ << "<title>" << title_ << "</title>\n";
                file_ << "<style>\n";
                file_ << "table { border-collapse: collapse; width: 100%; }\n";
                file_ << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
                file_ << "th { background-color: #f2f2f2; }\n";
                file_ << "</style>\n";
                file_ << "</head>\n<body>\n";
                file_ << "<h1>" << title_ << "</h1>\n";
                file_ << "<p>Generated at: " << get_current_time_string() << "</p>\n";
                file_ << "<table>\n<tr><th>Timestamp</th><th>Level</th><th>Logger</th><th>Message</th></tr>\n";
                break;
        }
    }
    
    void write_footer() {
        switch (format_) {
            case ReportFormat::TEXT:
                file_ << "\n=== End of Report ===\n";
                break;
            case ReportFormat::CSV:
                // CSV不需要特殊结尾
                break;
            case ReportFormat::JSON:
                file_ << "  ]\n}\n";
                break;
            case ReportFormat::HTML:
                file_ << "</table>\n</body>\n</html>\n";
                break;
        }
    }
    
    std::string get_current_time_string() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

ReportGenerator::ReportGenerator() : impl_(std::make_unique<Impl>()) {}
ReportGenerator::~ReportGenerator() = default;

bool ReportGenerator::set_output_file(const std::string& filepath) {
    return impl_->set_output_file(filepath);
}

void ReportGenerator::set_format(ReportFormat format) {
    impl_->set_format(format);
}

void ReportGenerator::set_title(const std::string& title) {
    impl_->set_title(title);
}

bool ReportGenerator::generate_entries_report(const std::vector<LogEntry>& entries) {
    return impl_->generate_entries_report(entries);
}

bool ReportGenerator::generate_statistics_report(const LogStatistics& statistics) {
    return impl_->generate_statistics_report(statistics);
}

bool ReportGenerator::generate_comprehensive_report(const std::vector<LogEntry>& entries, 
                                                 const LogStatistics& statistics) {
    return impl_->generate_comprehensive_report(entries, statistics);
}

bool ReportGenerator::generate_filtered_report(const std::vector<LogEntry>& entries,
                                             const LogFilter& filter) {
    return impl_->generate_filtered_report(entries, filter);
}

void ReportGenerator::close() {
    impl_->close();
}

bool ReportGenerator::is_open() const {
    return impl_->is_open();
}

// ConsoleReportGenerator 实现
ConsoleReportGenerator::ConsoleReportGenerator() : color_enabled_(true) {}

void ConsoleReportGenerator::print_entries(const std::vector<LogEntry>& entries, size_t max_count) {
    size_t count = 0;
    for (const auto& entry : entries) {
        if (count >= max_count) {
            std::cout << "... and " << (entries.size() - max_count) << " more entries\n";
            break;
        }
        
        if (color_enabled_) {
            std::cout << get_level_color(entry.level);
        }
        
        std::cout << entry.to_string();
        
        if (color_enabled_) {
            std::cout << get_reset_color();
        }
        
        std::cout << "\n";
        ++count;
    }
}

void ConsoleReportGenerator::print_statistics(const LogStatistics& statistics) {
    std::cout << statistics.generate_report();
}

void ConsoleReportGenerator::print_filtered(const std::vector<LogEntry>& entries, const LogFilter& filter) {
    auto filtered = filter.filter(entries);
    std::cout << "Filtered " << filtered.size() << " out of " << entries.size() << " entries:\n";
    print_entries(filtered);
}

void ConsoleReportGenerator::set_color_enabled(bool enabled) {
    color_enabled_ = enabled;
}

std::string ConsoleReportGenerator::get_level_color(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "\033[37m"; // White
        case LogLevel::DEBUG: return "\033[36m"; // Cyan
        case LogLevel::INFO:  return "\033[32m"; // Green
        case LogLevel::WARN:  return "\033[33m"; // Yellow
        case LogLevel::ERROR: return "\033[31m"; // Red
        case LogLevel::FATAL: return "\033[35m"; // Magenta
        default: return "\033[0m"; // Reset
    }
}

std::string ConsoleReportGenerator::get_reset_color() {
    return "\033[0m";
}

} // namespace loganalyzer