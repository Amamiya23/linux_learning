/**
 * 文件锁示例 - 阶段4：高级文件操作
 * 学习目标：掌握 flock() 和 fcntl() 文件锁
 * 
 * 编译: g++ -Wall -Wextra -o file_lock 04_advanced/file_lock.cpp
 * 运行: 
 *   终端1: ./file_lock write test.txt
 *   终端2: ./file_lock write test.txt (会等待锁)
 */

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <cstring>
#include <cerrno>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

/**
 * 使用 flock() 进行文件锁定
 * flock() 是咨询性锁，不同进程之间协作使用
 */
void flock_demo(const char* filepath, bool exclusive) {
    int fd = open(filepath, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        std::cerr << "无法打开文件: " << strerror(errno) << std::endl;
        return;
    }
    
    // 锁定类型
    int lock_type = exclusive ? LOCK_EX : LOCK_SH;
    
    std::cout << (exclusive ? "请求排他锁..." : "请求共享锁...") << std::endl;
    
    // flock() - 阻塞模式
    if (flock(fd, lock_type) == -1) {
        std::cerr << "锁定失败: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    std::cout << "✓ 锁定成功!" << std::endl;
    
    // 模拟工作
    std::cout << "正在执行文件操作..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // 解锁
    if (flock(fd, LOCK_UN) == -1) {
        std::cerr << "解锁失败: " << strerror(errno) << std::endl;
    } else {
        std::cout << "✓ 解锁成功!" << std::endl;
    }
    
    close(fd);
}

/**
 * 使用 fcntl() 进行记录锁定
 * 更加精细的锁控制，可以锁定文件的一部分
 */
void fcntl_demo(const char* filepath) {
    int fd = open(filepath, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        std::cerr << "无法打开文件: " << strerror(errno) << std::endl;
        return;
    }
    
    // 写入测试数据
    const char* data = "0123456789ABCDEF";
    write(fd, data, strlen(data));
    
    // 锁定文件的前5个字节
    struct flock lock;
    lock.l_type = F_WRLCK;   // 排他锁
    lock.l_whence = SEEK_SET; // 从文件开头
    lock.l_start = 0;        // 偏移0
    lock.l_len = 5;          // 锁定5字节
    lock.l_pid = getpid();   // 进程ID
    
    std::cout << "尝试锁定文件字节 [0, 5)..." << std::endl;
    
    // F_SETLKW - 阻塞模式，等待锁可用
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        std::cerr << "锁定失败: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    std::cout << "✓ 锁定成功! 锁定范围: [0, 5)" << std::endl;
    std::cout << "当前进程PID: " << getpid() << std::endl;
    
    // 显示当前锁信息
    lock.l_type = F_GETLK;
    if (fcntl(fd, F_GETLK, &lock) == 0) {
        if (lock.l_type == F_UNLCK) {
            std::cout << "当前无锁" << std::endl;
        } else {
            std::cout << "被PID " << lock.l_pid << " 的进程锁定" << std::endl;
        }
    }
    
    // 模拟工作
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 解锁
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        std::cerr << "解锁失败: " << strerror(errno) << std::endl;
    } else {
        std::cout << "✓ 解锁成功!" << std::endl;
    }
    
    close(fd);
}

/**
 * 多进程锁竞争模拟（使用线程模拟）
 */
void lock_contention_demo(const char* filepath) {
    std::vector<std::thread> threads;
    
    // 启动多个写线程（竞争排他锁）
    for (int i = 0; i < 3; i++) {
        threads.emplace_back([filepath, i]() {
            std::cout << "[线程" << i << "] 尝试获取排他锁..." << std::endl;
            
            int fd = open(filepath, O_RDWR);
            if (fd == -1) {
                return;
            }
            
            if (flock(fd, LOCK_EX) == 0) {
                std::cout << "[线程" << i << "] ✓ 获得排他锁!" << std::endl;
                
                // 写入数据
                std::string msg = "线程" + std::to_string(i) + "写入\n";
                write(fd, msg.c_str(), msg.length());
                
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                flock(fd, LOCK_UN);
                std::cout << "[线程" << i << "] 解锁" << std::endl;
            }
            
            close(fd);
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
}

/**
 * 死锁避免示例 - 顺序锁定
 */
void deadlock_demo() {
    const char* file1 = "lock1.txt";
    const char* file2 = "lock2.txt";
    
    // 创建测试文件
    int fd1 = open(file1, O_CREAT | O_WRONLY, 0644);
    int fd2 = open(file2, O_CREAT | O_WRONLY, 0644);
    close(fd1);
    close(fd2);
    
    std::cout << "\n===== 死锁避免演示 =====" << std::endl;
    std::cout << "策略: 始终按固定顺序锁定文件" << std::endl;
    
    // 线程1: 先锁file1，再锁file2
    std::thread t1([file1, file2]() {
        int fd1 = open(file1, O_RDWR);
        int fd2 = open(file2, O_RDWR);
        
        std::cout << "[线程A] 锁定 " << file1 << std::endl;
        flock(fd1, LOCK_EX);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "[线程A] 锁定 " << file2 << std::endl;
        flock(fd2, LOCK_EX);
        
        std::cout << "[线程A] 完成工作" << std::endl;
        
        flock(fd1, LOCK_UN);
        flock(fd2, LOCK_UN);
        close(fd1);
        close(fd2);
    });
    
    // 线程2: 按相同顺序锁定（避免死锁）
    std::thread t2([file1, file2]() {
        int fd1 = open(file1, O_RDWR);
        int fd2 = open(file2, O_RDWR);
        
        std::cout << "[线程B] 锁定 " << file1 << std::endl;
        flock(fd1, LOCK_EX);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "[线程B] 锁定 " << file2 << std::endl;
        flock(fd2, LOCK_EX);
        
        std::cout << "[线程B] 完成工作" << std::endl;
        
        flock(fd1, LOCK_UN);
        flock(fd2, LOCK_UN);
        close(fd1);
        close(fd2);
    });
    
    t1.join();
    t2.join();
    
    std::cout << "所有线程完成!" << std::endl;
}

/**
 * 打印帮助
 */
void print_help(const char* prog) {
    std::cout << "用法: " << prog << " <模式> [文件]\n"
              << "\n模式:\n"
              << "  flock     - 使用flock()锁定文件\n"
              << "  fcntl     - 使用fcntl()记录锁\n"
              << "  contend   - 多线程锁竞争演示\n"
              << "  deadlock  - 死锁避免演示\n"
              << "\n示例:\n"
              << "  " << prog << " flock test.txt\n"
              << "  " << prog << " fcntl test.txt\n"
              << "  " << prog << " contend test.txt\n"
              << "  " << prog << " deadlock\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }
    
    std::string mode = argv[1];
    const char* filepath = (argc > 2) ? argv[2] : "lock_test.txt";
    
    if (mode == "flock") {
        flock_demo(filepath, true);
    } else if (mode == "flock-sh") {
        flock_demo(filepath, false);
    } else if (mode == "fcntl") {
        fcntl_demo(filepath);
    } else if (mode == "contend") {
        lock_contention_demo(filepath);
    } else if (mode == "deadlock") {
        deadlock_demo();
    } else {
        std::cerr << "未知模式: " << mode << std::endl;
        print_help(argv[0]);
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
