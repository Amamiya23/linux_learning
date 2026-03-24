/**
 * 日志系统测试程序
 * 
 * 编译: cd linux-fileio/05_projects/logger
 *       g++ -std=c++17 -I../.. main.cpp logger.cpp -o logger_demo
 * 运行: ./logger_demo
 */

#include <iostream>
#include <thread>
#include <chrono>
#include "logger.h"

int main() {
    std::cout << "===== 日志系统演示 =====" << std::endl;
    
    // 初始化日志系统
    Logger& logger = Logger::instance();
    
    if (!logger.init("./logs", "app", 1024 * 1024, 3)) {
        std::cerr << "日志系统初始化失败" << std::endl;
        return 1;
    }
    
    // 设置日志级别
    logger.set_level(LogLevel::DEBUG);
    
    // 写入不同级别的日志
    logger.debug("这是一条调试信息");
    logger.info("这是一条普通信息");
    logger.warning("这是一条警告信息");
    logger.error("这是一条错误信息");
    logger.fatal("这是一条致命错误");
    
    // 模拟多线程写入
    std::cout << "\n启动多线程写入测试..." << std::endl;
    
    auto thread_func = [](int id) {
        Logger& log = Logger::instance();
        for (int i = 0; i < 5; i++) {
            log.info("线程 " + std::to_string(id) + " 写入第 " + std::to_string(i) + " 条日志");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };
    
    std::thread t1(thread_func, 1);
    std::thread t2(thread_func, 2);
    
    t1.join();
    t2.join();
    
    std::cout << "\n日志文件已写入到 ./logs 目录" << std::endl;
    std::cout << "查看日志文件: cat logs/app.*.log" << std::endl;
    
    // 关闭日志系统
    logger.shutdown();
    
    return 0;
}
