#include "../../include/log_parser.h"
#include "../../include/file_reader.h"
#include "../../include/log_filter.h"
#include "../../include/report_generator.h"
#include "../../include/directory_walker.h"
#include <iostream>
#include <string>
#include <regex>

using namespace loganalyzer;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <file_or_directory> <pattern> [options]\n";
        std::cerr << "Options:\n";
        std::cerr << "  -l, --level <level>  Filter by minimum level\n";
        std::cerr << "  -i, --ignore-case    Ignore case in pattern matching\n";
        std::cerr << "  -r, --recursive      Process directories recursively\n";
        std::cerr << "  -n, --count <count>  Maximum number of results (default: 100)\n";
        return 1;
    }
    
    std::string input_path = argv[1];
    std::string pattern = argv[2];
    LogLevel min_level = LogLevel::TRACE;
    bool ignore_case = false;
    bool recursive = false;
    size_t max_results = 100;
    
    // 解析选项
    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-l" || arg == "--level") {
            if (i + 1 < argc) {
                min_level = string_to_level(argv[++i]);
            }
        } else if (arg == "-i" || arg == "--ignore-case") {
            ignore_case = true;
        } else if (arg == "-r" || arg == "--recursive") {
            recursive = true;
        } else if (arg == "-n" || arg == "--count") {
            if (i + 1 < argc) {
                max_results = std::stoul(argv[++i]);
            }
        }
    }
    
    // 检查输入路径
    if (!FileReader::file_exists(input_path)) {
        std::cerr << "Error: Input path does not exist: " << input_path << std::endl;
        return 1;
    }
    
    // 编译正则表达式
    std::regex regex_pattern;
    try {
        if (ignore_case) {
            regex_pattern = std::regex(pattern, std::regex_constants::icase);
        } else {
            regex_pattern = std::regex(pattern);
        }
    } catch (const std::regex_error&) {
        std::cerr << "Error: Invalid regex pattern: " << pattern << std::endl;
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
    FilterCriteria criteria;
    criteria.min_level = min_level;
    criteria.message_pattern = regex_pattern;
    
    LogFilter filter;
    filter.set_criteria(criteria);
    
    auto filtered_entries = filter.filter(all_entries);
    
    // 输出结果
    ConsoleReportGenerator console_gen;
    console_gen.set_color_enabled(true);
    
    std::cout << "Found " << filtered_entries.size() << " matching entries:\n";
    console_gen.print_entries(filtered_entries, max_results);
    
    return 0;
}