/**
 * 内存映射示例 - 阶段4：高级文件操作
 * 学习目标：掌握 mmap() 和 munmap() 内存映射文件
 * 
 * 编译: g++ -Wall -Wextra -o memory_map 04_advanced/memory_map.cpp
 * 运行: ./memory_map
 */

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cstring>
#include <cerrno>
#include <string>
#include <chrono>

/**
 * 基础内存映射示例
 */
void basic_mmap_demo(const char* filepath) {
    std::cout << "===== 基础内存映射演示 =====" << std::endl;
    
    // 创建测试文件并写入数据
    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "无法创建文件: " << strerror(errno) << std::endl;
        return;
    }
    
    const char* original_data = "Hello, Memory Mapped File!";
    size_t file_size = strlen(original_data);
    
    // 扩展文件大小
    if (ftruncate(fd, file_size) == -1) {
        std::cerr << "无法扩展文件: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    // 写入原始数据
    write(fd, original_data, file_size);
    
    // 内存映射文件
    // void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
    void* mapped = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (mapped == MAP_FAILED) {
        std::cerr << "内存映射失败: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    std::cout << "✓ 文件映射到内存地址: " << mapped << std::endl;
    
    // 通过内存指针访问文件内容（像访问数组一样）
    char* data = static_cast<char*>(mapped);
    
    std::cout << "原始内容: " << data << std::endl;
    
    // 修改内存（等同于写入文件）
    strcpy(data, "Modified via mmap!");
    
    // 同步内存到磁盘
    if (msync(mapped, file_size, MS_SYNC) == -1) {
        std::cerr << "同步失败: " << strerror(errno) << std::endl;
    } else {
        std::cout << "✓ 数据已同步到磁盘" << std::endl;
    }
    
    // 验证文件内容
    close(fd);
    fd = open(filepath, O_RDONLY);
    char verify[256];
    read(fd, verify, sizeof(verify));
    std::cout << "验证文件内容: " << verify << std::endl;
    close(fd);
    
    // 解除映射
    munmap(mapped, file_size);
    std::cout << "✓ 解除内存映射" << std::endl;
}

/**
 * 大文件高效读取示例
 */
void large_file_demo() {
    std::cout << "\n===== 大文件高效读取演示 =====" << std::endl;
    
    const char* big_file = "big_data.dat";
    const size_t file_size = 1024 * 1024; // 1MB
    
    // 创建测试大文件
    std::cout << "创建测试文件 (" << file_size / 1024 << " KB)..." << std::endl;
    int fd = open(big_file, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "无法创建文件: " << strerror(errno) << std::endl;
        return;
    }
    
    // 扩展文件
    ftruncate(fd, file_size);
    
    // 映射整个文件
    void* mapped = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        std::cerr << "映射失败: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    // 初始化数据（模拟写入）
    char* data = static_cast<char*>(mapped);
    for (size_t i = 0; i < file_size; i++) {
        data[i] = static_cast<char>('A' + (i % 26));
    }
    
    // 同步
    msync(mapped, file_size, MS_SYNC);
    
    // 通过内存访问 - 随机访问示例
    std::cout << "随机访问测试..." << std::endl;
    for (int i = 0; i < 5; i++) {
        size_t random_pos = (file_size / 5) * i;
        std::cout << "位置 " << random_pos << ": " << data[random_pos] << std::endl;
    }
    
    // 对比：普通读取 vs 内存映射
    std::cout << "\n性能对比..." << std::endl;
    
    // 普通读取方式
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 10; iter++) {
        lseek(fd, 0, SEEK_SET);
        char buf[4096];
        for (size_t pos = 0; pos < file_size; pos += sizeof(buf)) {
            read(fd, buf, sizeof(buf));
        }
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
    
    // 内存映射方式
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 10; iter++) {
        for (size_t pos = 0; pos < file_size; pos += 4096) {
            volatile char c = data[pos];
            (void)c; // 防止优化
        }
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
    
    std::cout << "普通读取: " << duration1.count() << " 微秒" << std::endl;
    std::cout << "内存映射: " << duration2.count() << " 微秒" << std::endl;
    std::cout << "加速比:   " << (double)duration1.count() / duration2.count() << "x" << std::endl;
    
    // 清理
    munmap(mapped, file_size);
    close(fd);
    
    // 删除测试文件
    unlink(big_file);
}

/**
 * 私有映射示例 - 用于进程间共享数据
 */
void private_mapping_demo() {
    std::cout << "\n===== 私有映射演示 =====" << std::endl;
    
    const char* shm_file = "shared.dat";
    size_t page_size = sysconf(_SC_PAGESIZE);
    
    // 创建文件
    int fd = open(shm_file, O_RDWR | O_CREAT | O_TRUNC, 0644);
    ftruncate(fd, page_size);
    
    // MAP_SHARED - 映射的修改会影响文件
    void* shared = mmap(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    // MAP_PRIVATE - 映射的修改不会影响文件（copy-on-write）
    void* private_map = mmap(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    
    if (shared == MAP_FAILED || private_map == MAP_FAILED) {
        std::cerr << "映射失败" << std::endl;
        if (shared != MAP_FAILED) munmap(shared, page_size);
        if (private_map != MAP_FAILED) munmap(private_map, page_size);
        close(fd);
        return;
    }
    
    // 写入共享映射
    strcpy(static_cast<char*>(shared), "Shared content");
    
    // 写入私有映射
    strcpy(static_cast<char*>(private_map), "Private content");
    
    // 验证
    std::cout << "共享映射内容: " << static_cast<char*>(shared) << std::endl;
    std::cout << "私有映射内容: " << static_cast<char*>(private_map) << std::endl;
    
    // 再次读取文件内容
    char file_content[256];
    lseek(fd, 0, SEEK_SET);
    read(fd, file_content, 256);
    std::cout << "文件实际内容: " << file_content << std::endl;
    std::cout << "(私有映射修改未反映到文件中)" << std::endl;
    
    // 清理
    munmap(shared, page_size);
    munmap(private_map, page_size);
    close(fd);
    unlink(shm_file);
}

/**
 * 进程间通信示例
 */
void ipc_demo() {
    std::cout << "\n===== 进程间通信演示 =====" << std::endl;
    
    const char* ipc_file = "ipc_memory.dat";
    size_t page_size = sysconf(_SC_PAGESIZE);
    
    // 创建共享内存文件
    int fd = open(ipc_file, O_RDWR | O_CREAT | O_TRUNC, 0644);
    ftruncate(fd, page_size);
    
    // 创建共享映射
    void* shared = mmap(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        std::cerr << "映射失败" << std::endl;
        close(fd);
        return;
    }
    
    // 初始化共享数据
    struct SharedData {
        int message_count;
        char latest_message[256];
    };
    
    SharedData* data = static_cast<SharedData*>(shared);
    data->message_count = 0;
    strcpy(data->latest_message, "Init");
    
    // Fork子进程
    pid_t pid = fork();
    
    if (pid == 0) {
        // 子进程：写入数据
        for (int i = 0; i < 5; i++) {
            data->message_count++;
            snprintf(data->latest_message, sizeof(data->latest_message), 
                    "Message from child: %d", i);
            std::cout << "[子进程] 写入: " << data->latest_message << std::endl;
            msync(shared, page_size, MS_ASYNC);
            usleep(100000);
        }
        _exit(0);
    } else {
        // 父进程：读取数据
        sleep(1); // 等待子进程开始
        for (int i = 0; i < 5; i++) {
            std::cout << "[父进程] 读取: " << data->latest_message 
                      << " (count: " << data->message_count << ")" << std::endl;
            usleep(100000);
        }
        wait(nullptr);
    }
    
    // 清理
    munmap(shared, page_size);
    close(fd);
    unlink(ipc_file);
}

int main(int argc, char* argv[]) {
    std::cout << "内存映射文件示例 - Linux文件IO学习" << std::endl;
    std::cout << "页面大小: " << sysconf(_SC_PAGESIZE) << " 字节" << std::endl;
    std::cout << "==================================" << std::endl;
    
    basic_mmap_demo("mmap_test.txt");
    large_file_demo();
    private_mapping_demo();
    ipc_demo();
    
    std::cout << "\n所有演示完成!" << std::endl;
    
    return EXIT_SUCCESS;
}
