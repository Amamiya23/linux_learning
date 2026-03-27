#include "../include/directory_walker.h"
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>

namespace loganalyzer {

// DirectoryWalker 实现
class DirectoryWalker::Impl {
public:
    Impl() : recursive_(true) {}
    
    bool set_directory(const std::string& dirpath) {
        if (!is_directory(dirpath)) {
            return false;
        }
        dirpath_ = dirpath;
        return true;
    }
    
    void set_file_filter(std::function<bool(const FileInfo&)> filter) {
        filter_ = filter;
    }
    
    void set_recursive(bool recursive) {
        recursive_ = recursive;
    }
    
    void set_extension_filter(const std::vector<std::string>& extensions) {
        extensions_ = extensions;
    }
    
    std::vector<FileInfo> walk() {
        std::vector<FileInfo> results;
        if (dirpath_.empty()) {
            return results;
        }
        
        walk_directory(dirpath_, results);
        return results;
    }
    
    void walk(std::function<bool(const FileInfo&)> callback) {
        if (dirpath_.empty() || !callback) {
            return;
        }
        
        auto files = walk();
        for (const auto& file : files) {
            if (!callback(file)) {
                break;
            }
        }
    }
    
    std::vector<FileInfo> get_log_files(const std::string& dirpath) {
        if (!set_directory(dirpath)) {
            return {};
        }
        
        // 设置日志文件扩展名过滤器
        set_extension_filter({".log", ".txt", ".out"});
        
        return walk();
    }
    
    static bool is_directory(const std::string& path) {
        struct stat st;
        if (stat(path.c_str(), &st) != 0) {
            return false;
        }
        return S_ISDIR(st.st_mode);
    }
    
    static bool is_regular_file(const std::string& path) {
        struct stat st;
        if (stat(path.c_str(), &st) != 0) {
            return false;
        }
        return S_ISREG(st.st_mode);
    }
    
    static std::string get_extension(const std::string& filepath) {
        size_t pos = filepath.find_last_of('.');
        if (pos == std::string::npos) {
            return "";
        }
        return filepath.substr(pos);
    }
    
    static std::string get_filename(const std::string& filepath) {
        size_t pos = filepath.find_last_of('/');
        if (pos == std::string::npos) {
            return filepath;
        }
        return filepath.substr(pos + 1);
    }
    
    static std::string get_directory(const std::string& filepath) {
        size_t pos = filepath.find_last_of('/');
        if (pos == std::string::npos) {
            return "";
        }
        return filepath.substr(0, pos);
    }
    
    static std::string join_path(const std::string& dir, const std::string& file) {
        if (dir.empty()) {
            return file;
        }
        if (file.empty()) {
            return dir;
        }
        
        std::string result = dir;
        if (result.back() != '/') {
            result += '/';
        }
        result += file;
        return result;
    }
    
private:
    std::string dirpath_;
    std::function<bool(const FileInfo&)> filter_;
    std::vector<std::string> extensions_;
    bool recursive_;
    
    void walk_directory(const std::string& dirpath, std::vector<FileInfo>& results) {
        DIR* dir = opendir(dirpath.c_str());
        if (!dir) {
            return;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            
            // 跳过 . 和 ..
            if (name == "." || name == "..") {
                continue;
            }
            
            std::string fullpath = join_path(dirpath, name);
            FileInfo info = get_file_info(fullpath);
            
            // 应用扩展名过滤器
            if (!extensions_.empty() && info.is_regular_file) {
                std::string ext = get_extension(info.filename);
                bool found = false;
                for (const auto& allowed_ext : extensions_) {
                    if (ext == allowed_ext) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    continue;
                }
            }
            
            // 应用自定义过滤器
            if (filter_ && !filter_(info)) {
                continue;
            }
            
            results.push_back(info);
            
            // 递归处理子目录
            if (info.is_directory && recursive_) {
                walk_directory(fullpath, results);
            }
        }
        
        closedir(dir);
    }
    
    FileInfo get_file_info(const std::string& filepath) {
        FileInfo info;
        info.filepath = filepath;
        info.filename = get_filename(filepath);
        info.extension = get_extension(info.filename);
        
        struct stat st;
        if (stat(filepath.c_str(), &st) == 0) {
            info.file_size = st.st_size;
            info.modification_time = st.st_mtime;
            info.is_regular_file = S_ISREG(st.st_mode);
            info.is_directory = S_ISDIR(st.st_mode);
        } else {
            info.file_size = 0;
            info.modification_time = 0;
            info.is_regular_file = false;
            info.is_directory = false;
        }
        
        return info;
    }
};

DirectoryWalker::DirectoryWalker() : impl_(std::make_unique<Impl>()) {}
DirectoryWalker::~DirectoryWalker() = default;

bool DirectoryWalker::set_directory(const std::string& dirpath) {
    return impl_->set_directory(dirpath);
}

void DirectoryWalker::set_file_filter(std::function<bool(const FileInfo&)> filter) {
    impl_->set_file_filter(filter);
}

void DirectoryWalker::set_recursive(bool recursive) {
    impl_->set_recursive(recursive);
}

void DirectoryWalker::set_extension_filter(const std::vector<std::string>& extensions) {
    impl_->set_extension_filter(extensions);
}

std::vector<FileInfo> DirectoryWalker::walk() {
    return impl_->walk();
}

void DirectoryWalker::walk(std::function<bool(const FileInfo&)> callback) {
    impl_->walk(callback);
}

std::vector<FileInfo> DirectoryWalker::get_log_files(const std::string& dirpath) {
    return impl_->get_log_files(dirpath);
}

bool DirectoryWalker::is_directory(const std::string& path) {
    return Impl::is_directory(path);
}

bool DirectoryWalker::is_regular_file(const std::string& path) {
    return Impl::is_regular_file(path);
}

std::string DirectoryWalker::get_extension(const std::string& filepath) {
    return Impl::get_extension(filepath);
}

std::string DirectoryWalker::get_filename(const std::string& filepath) {
    return Impl::get_filename(filepath);
}

std::string DirectoryWalker::get_directory(const std::string& filepath) {
    return Impl::get_directory(filepath);
}

std::string DirectoryWalker::join_path(const std::string& dir, const std::string& file) {
    return Impl::join_path(dir, file);
}

// MultiFileProcessor 实现
MultiFileProcessor::MultiFileProcessor() : processed_count_(0) {}

void MultiFileProcessor::add_file(const std::string& filepath) {
    files_.push_back(filepath);
}

void MultiFileProcessor::add_directory(const std::string& dirpath, bool recursive) {
    DirectoryWalker walker;
    walker.set_recursive(recursive);
    walker.set_extension_filter({".log", ".txt", ".out"});
    
    auto files = walker.get_log_files(dirpath);
    for (const auto& file : files) {
        files_.push_back(file.filepath);
    }
}

void MultiFileProcessor::set_file_handler(std::function<bool(const std::string&)> handler) {
    handler_ = handler;
}

void MultiFileProcessor::process_all() {
    processed_count_ = 0;
    
    for (const auto& filepath : files_) {
        if (handler_) {
            if (!handler_(filepath)) {
                break;
            }
        }
        ++processed_count_;
    }
}

size_t MultiFileProcessor::get_file_count() const {
    return files_.size();
}

size_t MultiFileProcessor::get_processed_count() const {
    return processed_count_;
}

void MultiFileProcessor::clear() {
    files_.clear();
    processed_count_ = 0;
}

} // namespace loganalyzer