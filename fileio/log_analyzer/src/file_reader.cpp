#include "../include/file_reader.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

namespace loganalyzer {

// FileReader 静态方法实现
bool FileReader::file_exists(const std::string& filepath) {
    struct stat st;
    return get_file_stat(filepath, st);
}

bool FileReader::get_file_stat(const std::string& filepath, struct stat& st) {
    return stat(filepath.c_str(), &st) == 0;
}

// StandardFileReader 实现
class StandardFileReader::Impl {
public:
    Impl() : file_size_(0), is_open_(false) {}
    
    bool open(const std::string& filepath) {
        filepath_ = filepath;
        file_.open(filepath, std::ios::in);
        if (!file_.is_open()) {
            return false;
        }
        
        // 获取文件大小
        file_.seekg(0, std::ios::end);
        file_size_ = file_.tellg();
        file_.seekg(0, std::ios::beg);
        
        is_open_ = true;
        return true;
    }
    
    void close() {
        if (file_.is_open()) {
            file_.close();
        }
        is_open_ = false;
    }
    
    std::vector<std::string> read_all_lines() {
        std::vector<std::string> lines;
        if (!is_open_) {
            return lines;
        }
        
        std::string line;
        while (std::getline(file_, line)) {
            // 移除行尾的回车符（Windows格式）
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            lines.push_back(line);
        }
        
        return lines;
    }
    
    void read_lines(std::function<bool(const std::string&, size_t)> callback) {
        if (!is_open_ || !callback) {
            return;
        }
        
        std::string line;
        size_t line_number = 1;
        
        while (std::getline(file_, line)) {
            // 移除行尾的回车符
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            if (!callback(line, line_number)) {
                break;
            }
            ++line_number;
        }
    }
    
    bool is_open() const {
        return is_open_;
    }
    
    size_t get_file_size() const {
        return file_size_;
    }
    
private:
    std::ifstream file_;
    std::string filepath_;
    size_t file_size_;
    bool is_open_;
};

StandardFileReader::StandardFileReader() : impl_(std::make_unique<Impl>()) {}
StandardFileReader::~StandardFileReader() = default;

bool StandardFileReader::open(const std::string& filepath) {
    filepath_ = filepath;
    return impl_->open(filepath);
}

void StandardFileReader::close() {
    impl_->close();
}

std::vector<std::string> StandardFileReader::read_all_lines() {
    return impl_->read_all_lines();
}

void StandardFileReader::read_lines(std::function<bool(const std::string&, size_t)> callback) {
    impl_->read_lines(callback);
}

bool StandardFileReader::is_open() const {
    return impl_->is_open();
}

size_t StandardFileReader::get_file_size() const {
    return impl_->get_file_size();
}

// MMapFileReader 实现
class MMapFileReader::Impl {
public:
    Impl() : fd_(-1), data_(MAP_FAILED), file_size_(0), is_open_(false) {}
    
    ~Impl() {
        close();
    }
    
    bool open(const std::string& filepath) {
        filepath_ = filepath;
        
        // 打开文件
        fd_ = ::open(filepath.c_str(), O_RDONLY);
        if (fd_ == -1) {
            std::cerr << "Failed to open file: " << strerror(errno) << std::endl;
            return false;
        }
        
        // 获取文件大小
        struct stat st;
        if (fstat(fd_, &st) == -1) {
            std::cerr << "Failed to get file size: " << strerror(errno) << std::endl;
            ::close(fd_);
            fd_ = -1;
            return false;
        }
        file_size_ = st.st_size;
        
        if (file_size_ == 0) {
            // 空文件，不进行内存映射
            is_open_ = true;
            return true;
        }
        
        // 内存映射
        data_ = mmap(nullptr, file_size_, PROT_READ, MAP_PRIVATE, fd_, 0);
        if (data_ == MAP_FAILED) {
            std::cerr << "Failed to mmap file: " << strerror(errno) << std::endl;
            ::close(fd_);
            fd_ = -1;
            return false;
        }
        
        is_open_ = true;
        return true;
    }
    
    void close() {
        if (data_ != MAP_FAILED && data_ != nullptr) {
            munmap(data_, file_size_);
            data_ = MAP_FAILED;
        }
        
        if (fd_ != -1) {
            ::close(fd_);
            fd_ = -1;
        }
        
        is_open_ = false;
        file_size_ = 0;
    }
    
    std::vector<std::string> read_all_lines() {
        std::vector<std::string> lines;
        if (!is_open_ || file_size_ == 0) {
            return lines;
        }
        
        const char* start = static_cast<const char*>(data_);
        const char* end = start + file_size_;
        const char* line_start = start;
        
        while (line_start < end) {
            const char* line_end = line_start;
            
            // 查找行尾
            while (line_end < end && *line_end != '\n') {
                ++line_end;
            }
            
            // 创建字符串
            std::string line(line_start, line_end);
            
            // 移除行尾的回车符
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            lines.push_back(line);
            
            // 移动到下一行
            line_start = line_end + 1;
        }
        
        return lines;
    }
    
    void read_lines(std::function<bool(const std::string&, size_t)> callback) {
        if (!is_open_ || file_size_ == 0 || !callback) {
            return;
        }
        
        const char* start = static_cast<const char*>(data_);
        const char* end = start + file_size_;
        const char* line_start = start;
        size_t line_number = 1;
        
        while (line_start < end) {
            const char* line_end = line_start;
            
            // 查找行尾
            while (line_end < end && *line_end != '\n') {
                ++line_end;
            }
            
            // 创建字符串
            std::string line(line_start, line_end);
            
            // 移除行尾的回车符
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            if (!callback(line, line_number)) {
                break;
            }
            
            ++line_number;
            line_start = line_end + 1;
        }
    }
    
    bool is_open() const {
        return is_open_;
    }
    
    size_t get_file_size() const {
        return file_size_;
    }
    
    const char* get_data() const {
        return static_cast<const char*>(data_);
    }
    
private:
    std::string filepath_;
    int fd_;
    void* data_;
    size_t file_size_;
    bool is_open_;
};

MMapFileReader::MMapFileReader() : impl_(std::make_unique<Impl>()) {}
MMapFileReader::~MMapFileReader() = default;

bool MMapFileReader::open(const std::string& filepath) {
    filepath_ = filepath;
    return impl_->open(filepath);
}

void MMapFileReader::close() {
    impl_->close();
}

std::vector<std::string> MMapFileReader::read_all_lines() {
    return impl_->read_all_lines();
}

void MMapFileReader::read_lines(std::function<bool(const std::string&, size_t)> callback) {
    impl_->read_lines(callback);
}

bool MMapFileReader::is_open() const {
    return impl_->is_open();
}

size_t MMapFileReader::get_file_size() const {
    return impl_->get_file_size();
}

const char* MMapFileReader::get_data() const {
    return impl_->get_data();
}

// FileReaderFactory 实现
std::unique_ptr<FileReader> FileReaderFactory::create_reader(ReaderType type) {
    switch (type) {
        case ReaderType::STANDARD:
            return std::make_unique<StandardFileReader>();
        case ReaderType::MMAP:
            return std::make_unique<MMapFileReader>();
        default:
            return std::make_unique<StandardFileReader>();
    }
}

std::unique_ptr<FileReader> FileReaderFactory::create_optimal_reader(const std::string& filepath) {
    struct stat st;
    if (!FileReader::get_file_stat(filepath, st)) {
        return create_reader(ReaderType::STANDARD);
    }
    
    // 对于大于1MB的文件使用mmap
    const size_t MMAP_THRESHOLD = 1024 * 1024; // 1MB
    
    if (st.st_size > MMAP_THRESHOLD) {
        return create_reader(ReaderType::MMAP);
    } else {
        return create_reader(ReaderType::STANDARD);
    }
}

} // namespace loganalyzer