#include "../../include/log_parser.h"
#include "../../include/file_reader.h"
#include "../../include/report_generator.h"
#include "../../include/directory_walker.h"
#include <iostream>
#include <string>

using namespace loganalyzer;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_or_directory> [options]\n";
        std::cerr << "Options:\n";
        std::cerr << "  -f, --format <format>  Output format: text, csv, json, html (default: text)\n";
        std::cerr << "  -o, --output <file>    Output file (default: stdout)\n";
        std::cerr << "  -r, --recursive        Process directories recursively\n";
        std::cerr << "  -t, --title <title>    Report title\n";
        return 1;
    }
    
    std::string input_path = argv[1];
    ReportFormat format = ReportFormat::TEXT;
    std::string output_file;
    bool recursive = false;
    std::string title = "Formatted Log Report";
    
    // 解析选项
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-f" || arg == "--format") {
            if (i + 1 < argc) {
                std::string format_str = argv[++i];
                if (format_str == "csv") format = ReportFormat::CSV;
                else if (format_str == "json") format = ReportFormat::JSON;
                else if (format_str == "html") format = ReportFormat::HTML;
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                output_file = argv[++i];
            }
        } else if (arg == "-r" || arg == "--recursive") {
            recursive = true;
        } else if (arg == "-t" || arg == "--title") {
            if (i + 1 < argc) {
                title = argv[++i];
            }
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
    
    // 收集统计信息
    LogStatistics statistics;
    statistics.collect(all_entries);
    
    // 生成报告
    if (output_file.empty()) {
        // 输出到控制台
        ConsoleReportGenerator console_gen;
        console_gen.set_color_enabled(true);
        
        if (format == ReportFormat::TEXT) {
            console_gen.print_entries(all_entries);
        } else {
            // 对于非文本格式，我们仍然输出到控制台，但格式可能不太好看
            ReportGenerator gen;
            gen.set_format(format);
            gen.set_title(title);
            // 这里我们需要一个输出到字符串的方法，但为了简化，我们输出到临时文件
            std::string temp_file = "/tmp/log_formatter_output.tmp";
            if (gen.set_output_file(temp_file)) {
                gen.generate_comprehensive_report(all_entries, statistics);
                gen.close();
                
                // 读取并输出临时文件
                auto reader = FileReaderFactory::create_reader(FileReaderFactory::ReaderType::STANDARD);
                if (reader->open(temp_file)) {
                    auto lines = reader->read_all_lines();
                    for (const auto& line : lines) {
                        std::cout << line << "\n";
                    }
                    reader->close();
                }
                
                // 删除临时文件
                remove(temp_file.c_str());
            }
        }
    } else {
        // 输出到文件
        ReportGenerator gen;
        if (!gen.set_output_file(output_file)) {
            std::cerr << "Error: Could not create output file: " << output_file << std::endl;
            return 1;
        }
        
        gen.set_format(format);
        gen.set_title(title);
        
        if (!gen.generate_comprehensive_report(all_entries, statistics)) {
            std::cerr << "Error: Failed to generate report\n";
            return 1;
        }
        
        gen.close();
        std::cout << "Report generated: " << output_file << std::endl;
    }
    
    return 0;
}