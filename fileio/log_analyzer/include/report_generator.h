#pragma once

#include "log_parser.h"
#include "log_filter.h"
#include <string>
#include <fstream>
#include <memory>

namespace loganalyzer {

// 报告格式
enum class ReportFormat {
    TEXT,    // 纯文本
    CSV,     // CSV格式
    JSON,    // JSON格式
    HTML     // HTML格式
};

// 报告生成器类
class ReportGenerator {
public:
    ReportGenerator();
    ~ReportGenerator();
    
    // 设置输出文件路径
    bool set_output_file(const std::string& filepath);
    
    // 设置报告格式
    void set_format(ReportFormat format);
    
    // 设置报告标题
    void set_title(const std::string& title);
    
    // 生成日志条目报告
    bool generate_entries_report(const std::vector<LogEntry>& entries);
    
    // 生成统计报告
    bool generate_statistics_report(const LogStatistics& statistics);
    
    // 生成综合报告
    bool generate_comprehensive_report(const std::vector<LogEntry>& entries, 
                                     const LogStatistics& statistics);
    
    // 生成过滤后的报告
    bool generate_filtered_report(const std::vector<LogEntry>& entries,
                                const LogFilter& filter);
    
    // 关闭输出文件
    void close();
    
    // 检查是否已打开
    bool is_open() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    
    // 格式化函数
    std::string format_entry_text(const LogEntry& entry);
    std::string format_entry_csv(const LogEntry& entry);
    std::string format_entry_json(const LogEntry& entry);
    std::string format_entry_html(const LogEntry& entry);
    
    // 写入文件头
    void write_header();
    
    // 写入文件尾
    void write_footer();
};

// 控制台报告生成器（输出到标准输出）
class ConsoleReportGenerator {
public:
    ConsoleReportGenerator();
    ~ConsoleReportGenerator() = default;
    
    // 打印日志条目
    void print_entries(const std::vector<LogEntry>& entries, size_t max_count = 100);
    
    // 打印统计信息
    void print_statistics(const LogStatistics& statistics);
    
    // 打印过滤后的日志
    void print_filtered(const std::vector<LogEntry>& entries, const LogFilter& filter);
    
    // 设置是否显示颜色
    void set_color_enabled(bool enabled);
    
private:
    bool color_enabled_;
    
    // 获取颜色代码
    std::string get_level_color(LogLevel level);
    std::string get_reset_color();
};

} // namespace loganalyzer