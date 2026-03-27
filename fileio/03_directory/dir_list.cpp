#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <vector>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <fcntl.h>

void error_exit(const std::string& msg) {
    std::cerr << msg << " - " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
}

void list_dir(const char* dirpath) {
    DIR* dir = opendir(dirpath);
    if (!dir) {
        error_exit("无法打开目录");
    }

    std::cout << "目录：" << dirpath << std::endl;
    std::cout<<"---------"<<std::endl;
    struct dirent* entry;
    int count = 0;

    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        std::cout << entry->d_name << std::endl;
        count++;
    }
    std::cout << "共 " << count << " 个条目" << std::endl;
}

void list_dir_tree(const char* dirpath, int depth = 0) {
    DIR* dir = opendir(dirpath);
    if (!dir) {
        error_exit("无法打开目录");
    }
    std::string indent(depth*2,' '); //根据深度创造空格
    struct dirent* entry;

    while((entry=readdir(dir))!=nullptr){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        std::string fullpath=std::string(dirpath)+"/"+entry->d_name;

        std::cout<<indent;
        if(entry->d_type==DT_DIR){
            std::cout<<entry->d_name<<"/"<<std::endl;
            list_dir_tree(fullpath.c_str(),depth+1);
        }
        else{
            std::cout<<entry->d_name<<std::endl;
        }
    }

}

int main(int argc, char* argv[]) {
    const char* dirpath = (argc > 1) ? argv[1] : ".";

    list_dir(dirpath);
    std::cout<<"---------------"<<std::endl;
    list_dir_tree(dirpath);
}