#include "../include/log_parser.h"
#include "../include/file_reader.h"
#include "../include/log_filter.h"
#include "../include/report_generator.h"
#include "../include/directory_walker.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace loganalyzer;

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options] <file_or_directory>\n";
    std::cout << "Options:\n";
    std::cout << "  -f, --format <format>    Output format: text, csv, json, html (default: text)\n";
    std::cout << "  -o, --output <file>      Output file (default: stdout)\n";
    std::cout << "  -l, --level <level>      Filter by minimum level: trace, debug, info, warn, error, fatal\n";
    std::cout << "  -L, --logger <name>      Filter by logger name\n";
    std::cout << "  -m, --message <pattern>  Filter by message pattern (regex)\n";
    std::cout << "  -s, --stats              Show statistics only\n";
    std::cout << "  -r, --recursive          Process directories recursively\n";
    std::cout << "  -c, --no-color           Disable colored output\n";
    std::cout << "  -h, --help               Show this help message\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << " app.log\n";
    std::cout << "  " << program_name << " -f json -o report.json /var/log\n";
    std::cout << "  " << program_name << " -l error -m \"database\" app.log\n";
}

LogLevel parse_level(const std::string& level_str) {
    return string_to_level(level_str);
}

ReportFormat parse_format(const std::string& format_str) {
    if (format_str == "csv") return ReportFormat::CSV;
    if (format_str == "json") return ReportFormat::JSON;
    if (format_str == "html") return ReportFormat::HTML;
    return ReportFormat::TEXT;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // 解析命令行参数
    std::string input_path;
    std::string output_file;
    ReportFormat format = ReportFormat::TEXT;
    FilterCriteria criteria;
    bool show_stats_only = false;
    bool recursive = false;
    bool color_enabled = true;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-f" || arg == "--format") {
            if (i + 1 < argc) {
                format = parse_format(argv[++i]);
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                output_file = argv[++i];
            }
        } else if (arg == "-l" || arg == "--level") {
            if (i + 1 < argc) {
                criteria.min_level = parse_level(argv[++i]);
            }
        } else if (arg == "-L" || arg == "--logger") {
            if (i + 1 < argc) {
                criteria.logger_name = argv[++i];
            }
        } else if (arg == "-m" || arg == "--message") {
            if (i + 1 < argc) {
                try {
                    criteria.message_pattern = std::regex(argv[++i]);
                } catch (const std::regex_error&) {
                    std::cerr << "Invalid regex pattern: " << argv[i] << std::endl;
                    return 1;
                }
            }
        } else if (arg == "-s" || arg == "--stats") {
            show_stats_only = true;
        } else if (arg == "-r" || arg == "--recursive") {
            recursive = true;
        } else if (arg == "-c" || arg == "--no-color") {
            color_enabled = false;
        } else if (arg[0] != '-') {
            input_path = arg;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (input_path.empty()) {
        std::cerr << "Error: No input file or directory specified\n";
        print_usage(argv[0]);
        return 1;
    }
    
    // 检查输入路径是否存在
    if (!FileReader::file_exists(input_path)) {
        std::cerr << "Error: Input path does not exist: " << input_path << std::endl;
        return 1;
    }
    
    // 收集所有日志条目
    std::vector<LogEntry> all_entries;
    LogParser parser;
    
    if (DirectoryWalker::is_directory(input_path)) {
        // 处理目录
        DirectoryWalker walker;
        walker.set_recursive(recursive);
        walker.set_extension_filter({".log", ".txt", ".out"});
        
        auto files = walker.get_log_files(input_path);
        std::cout << "Found " << files.size() << " log files\n";
        
        for (const auto& file_info : files) {
            auto reader = FileReaderFactory::create_optimal_reader(file_info.filepath);
            if (!reader->open(file_info.filepath)) {
                std::cerr << "Warning: Could not open file: " << file_info.filepath << std::endl;
                continue;
            }
            
            auto lines = reader->read_all_lines();
            auto entries = parser.parse_lines(lines);
            all_entries.insert(all_entries.end(), entries.begin(), entries.end());
            
            reader->close();
            std::cout << "Processed: " << file_info.filepath << " (" << entries.size() << " entries)\n";
        }
    } else {
        // 处理单个文件
        auto reader = FileReaderFactory::create_optimal_reader(input_path);
        if (!reader->open(input_path)) {
            std::cerr << "Error: Could not open file: " << input_path << std::endl;
            return 1;
        }
        
        auto lines = reader->read_all_lines();
        all_entries = parser.parse_lines(lines);
        reader->close();
        
        std::cout << "Parsed " << all_entries.size() << " log entries\n";
    }
    
    // 应用过滤器
    LogFilter filter;
    if (!criteria.is_empty()) {
        filter.set_criteria(criteria);
        all_entries = filter.filter(all_entries);
        std::cout << "Filtered to " << all_entries.size() << " entries\n";
    }
    
    // 收集统计信息
    LogStatistics statistics;
    statistics.collect(all_entries);
    
    // 生成报告
    if (show_stats_only) {
        ConsoleReportGenerator console_gen;
        console_gen.set_color_enabled(color_enabled);
        console_gen.print_statistics(statistics);
    } else {
        if (output_file.empty()) {
            // 输出到控制台
            ConsoleReportGenerator console_gen;
            console_gen.set_color_enabled(color_enabled);
            
            if (criteria.is_empty()) {
                console_gen.print_entries(all_entries);
            } else {
                console_gen.print_filtered(all_entries, filter);
            }
        } else {
            // 输出到文件
            ReportGenerator file_gen;
            if (!file_gen.set_output_file(output_file)) {
                std::cerr << "Error: Could not create output file: " << output_file << std::endl;
                return 1;
            }
            
            file_gen.set_format(format);
            file_gen.set_title("Log Analysis Report");
            
            if (!file_gen.generate_comprehensive_report(all_entries, statistics)) {
                std::cerr << "Error: Failed to generate report\n";
                return 1;
            }
            
            file_gen.close();
            std::cout << "Report generated: " << output_file << std::endl;
        }
    }
    
    return 0;
}