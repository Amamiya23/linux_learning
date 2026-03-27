#include "../../include/log_parser.h"
#include "../../include/file_reader.h"
#include "../../include/log_filter.h"
#include "../../include/report_generator.h"
#include "../../include/directory_walker.h"
#include <iostream>
#include <string>

using namespace loganalyzer;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_or_directory> [options]\n";
        std::cerr << "Options:\n";
        std::cerr << "  -l, --level <level>  Filter by minimum level\n";
        std::cerr << "  -r, --recursive      Process directories recursively\n";
        return 1;
    }
    
    std::string input_path = argv[1];
    LogLevel min_level = LogLevel::TRACE;
    bool recursive = false;
    
    // 解析选项
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-l" || arg == "--level") {
            if (i + 1 < argc) {
                min_level = string_to_level(argv[++i]);
            }
        } else if (arg == "-r" || arg == "--recursive") {
            recursive = true;
        }
    }
    
    // 检查输入路径
    if (!FileReader::file_exists(input_path)) {
        std::cerr << "Error: Input path does not exist: " << input_path << std::endl;
        return 1;
    }
    
    std::vector<LogEntry> all_entries;
    LogParser parser;
    
    if (DirectoryWalker::is_directory(input_path)) {
        DirectoryWalker walker;
        walker.set_recursive(recursive);
        walker.set_extension_filter({".log", ".txt", ".out"});
        
        auto files = walker.get_log_files(input_path);
        for (const auto& file_info : files) {
            auto reader = FileReaderFactory::create_optimal_reader(file_info.filepath);
            if (reader->open(file_info.filepath)) {
                auto lines = reader->read_all_lines();
                auto entries = parser.parse_lines(lines);
                all_entries.insert(all_entries.end(), entries.begin(), entries.end());
                reader->close();
            }
        }
    } else {
        auto reader = FileReaderFactory::create_optimal_reader(input_path);
        if (reader->open(input_path)) {
            auto lines = reader->read_all_lines();
            all_entries = parser.parse_lines(lines);
            reader->close();
        }
    }
    
    // 应用过滤器
    if (min_level != LogLevel::TRACE) {
        FilterCriteria criteria;
        criteria.min_level = min_level;
        LogFilter filter;
        filter.set_criteria(criteria);
        all_entries = filter.filter(all_entries);
    }
    
    // 生成统计信息
    LogStatistics statistics;
    statistics.collect(all_entries);
    
    // 输出统计信息
    ConsoleReportGenerator console_gen;
    console_gen.print_statistics(statistics);
    
    return 0;
}