#include "../include/log_parser.h"
#include <iostream>
#include <cassert>

using namespace loganalyzer;

void test_log_parser() {
    std::cout << "Testing LogParser...\n";
    
    LogParser parser;
    
    // 测试默认模式
    std::string line1 = "2023-01-01 12:00:00 [INFO] MyApp - Application started";
    auto entry1 = parser.parse_line(line1, 1);
    
    assert(entry1.has_value());
    assert(entry1->level == LogLevel::INFO);
    assert(entry1->logger_name == "MyApp");
    assert(entry1->message == "Application started");
    
    // 测试没有日志器名称的格式
    std::string line2 = "2023-01-01 12:00:01 ERROR Database connection failed";
    auto entry2 = parser.parse_line(line2, 2);
    
    assert(entry2.has_value());
    assert(entry2->level == LogLevel::ERROR);
    assert(entry2->message.find("Database") != std::string::npos);
    
    // 测试空行
    auto entry3 = parser.parse_line("", 3);
    assert(!entry3.has_value());
    
    // 测试无效格式
    auto entry4 = parser.parse_line("This is not a log line", 4);
    assert(!entry4.has_value());
    
    std::cout << "LogParser tests passed!\n";
}

void test_log_levels() {
    std::cout << "Testing log level conversion...\n";
    
    assert(string_to_level("TRACE") == LogLevel::TRACE);
    assert(string_to_level("DEBUG") == LogLevel::DEBUG);
    assert(string_to_level("INFO") == LogLevel::INFO);
    assert(string_to_level("WARN") == LogLevel::WARN);
    assert(string_to_level("WARNING") == LogLevel::WARN);
    assert(string_to_level("ERROR") == LogLevel::ERROR);
    assert(string_to_level("FATAL") == LogLevel::FATAL);
    assert(string_to_level("UNKNOWN") == LogLevel::UNKNOWN);
    assert(string_to_level("invalid") == LogLevel::UNKNOWN);
    
    assert(level_to_string(LogLevel::TRACE) == "TRACE");
    assert(level_to_string(LogLevel::DEBUG) == "DEBUG");
    assert(level_to_string(LogLevel::INFO) == "INFO");
    assert(level_to_string(LogLevel::WARN) == "WARN");
    assert(level_to_string(LogLevel::ERROR) == "ERROR");
    assert(level_to_string(LogLevel::FATAL) == "FATAL");
    assert(level_to_string(LogLevel::UNKNOWN) == "UNKNOWN");
    
    std::cout << "Log level tests passed!\n";
}

void test_log_entry() {
    std::cout << "Testing LogEntry...\n";
    
    LogEntry entry;
    entry.level = LogLevel::INFO;
    entry.logger_name = "TestLogger";
    entry.message = "Test message";
    entry.timestamp = std::chrono::system_clock::now();
    
    std::string str = entry.to_string();
    assert(str.find("INFO") != std::string::npos);
    assert(str.find("TestLogger") != std::string::npos);
    assert(str.find("Test message") != std::string::npos);
    
    std::cout << "LogEntry tests passed!\n";
}

int main() {
    std::cout << "Running log parser tests...\n\n";
    
    test_log_levels();
    test_log_entry();
    test_log_parser();
    
    std::cout << "\nAll tests passed!\n";
    return 0;
}