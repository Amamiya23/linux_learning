#include "../include/file_reader.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdio>

using namespace loganalyzer;

void create_test_file(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    file << content;
    file.close();
}

void test_standard_file_reader() {
    std::cout << "Testing StandardFileReader...\n";
    
    std::string test_file = "/tmp/test_file_reader.txt";
    std::string content = "Line 1\nLine 2\nLine 3\n";
    
    create_test_file(test_file, content);
    
    StandardFileReader reader;
    assert(reader.open(test_file));
    assert(reader.is_open());
    assert(reader.get_file_size() > 0);
    
    auto lines = reader.read_all_lines();
    assert(lines.size() == 3);
    assert(lines[0] == "Line 1");
    assert(lines[1] == "Line 2");
    assert(lines[2] == "Line 3");
    
    reader.close();
    assert(!reader.is_open());
    
    // 清理
    remove(test_file.c_str());
    
    std::cout << "StandardFileReader tests passed!\n";
}

void test_mmap_file_reader() {
    std::cout << "Testing MMapFileReader...\n";
    
    std::string test_file = "/tmp/test_mmap_file.txt";
    std::string content = "Line 1\nLine 2\nLine 3\n";
    
    create_test_file(test_file, content);
    
    MMapFileReader reader;
    assert(reader.open(test_file));
    assert(reader.is_open());
    assert(reader.get_file_size() > 0);
    
    auto lines = reader.read_all_lines();
    assert(lines.size() == 3);
    assert(lines[0] == "Line 1");
    assert(lines[1] == "Line 2");
    assert(lines[2] == "Line 3");
    
    reader.close();
    assert(!reader.is_open());
    
    // 清理
    remove(test_file.c_str());
    
    std::cout << "MMapFileReader tests passed!\n";
}

void test_file_exists() {
    std::cout << "Testing file_exists...\n";
    
    // 创建临时文件
    std::string test_file = "/tmp/test_exists.txt";
    create_test_file(test_file, "test");
    
    assert(FileReader::file_exists(test_file));
    assert(!FileReader::file_exists("/tmp/nonexistent_file.txt"));
    
    // 清理
    remove(test_file.c_str());
    
    std::cout << "file_exists tests passed!\n";
}

void test_factory() {
    std::cout << "Testing FileReaderFactory...\n";
    
    // 测试创建不同类型的读取器
    auto standard_reader = FileReaderFactory::create_reader(FileReaderFactory::ReaderType::STANDARD);
    assert(standard_reader != nullptr);
    
    auto mmap_reader = FileReaderFactory::create_reader(FileReaderFactory::ReaderType::MMAP);
    assert(mmap_reader != nullptr);
    
    // 测试自动选择读取器
    std::string test_file = "/tmp/test_factory.txt";
    create_test_file(test_file, "test content");
    
    auto optimal_reader = FileReaderFactory::create_optimal_reader(test_file);
    assert(optimal_reader != nullptr);
    
    // 清理
    remove(test_file.c_str());
    
    std::cout << "FileReaderFactory tests passed!\n";
}

int main() {
    std::cout << "Running file reader tests...\n\n";
    
    test_file_exists();
    test_standard_file_reader();
    test_mmap_file_reader();
    test_factory();
    
    std::cout << "\nAll tests passed!\n";
    return 0;
}