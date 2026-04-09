# Linux 进程与线程学习路线

这套内容面向已经学过文件 I/O、准备进入 Linux 进程和线程的 C++ 学习者。

目标不是只会背概念，而是通过运行、修改、观察代码来理解：

1. 进程是怎么创建出来的
2. 父子进程如何同步
3. 进程之间如何通信
4. 线程和进程的差别是什么
5. 多线程为什么会有竞争问题
6. 如何用互斥锁和条件变量解决并发问题

## 学习顺序

建议按下面顺序学习，不要跳步：

1. `01_process/fork_demo.cpp`
2. `02_process_wait/wait_demo.cpp`
3. `03_ipc_pipe/pipe_demo.cpp`
4. `04_threads/thread_demo.cpp`
5. `05_threads_mutex/counter_demo.cpp`
6. `06_threads_condition/producer_consumer.cpp`

## 学习方案

### 第一阶段：进程基础

学习重点：

1. `fork()` 会复制出什么
2. 父进程和子进程从哪里开始继续执行
3. `getpid()` 和 `getppid()` 的含义
4. 进程地址空间相互独立

练习方法：

1. 先运行 `fork_demo`
2. 观察父子进程输出顺序
3. 修改父子分支里的变量值，确认互不影响
4. 在父子分支里各自多打印几次，观察调度顺序

### 第二阶段：进程等待与回收

学习重点：

1. 为什么父进程要回收子进程
2. `waitpid()` 的作用
3. 如何判断子进程正常退出还是异常退出

练习方法：

1. 运行 `wait_demo`
2. 修改子进程退出码
3. 让子进程睡眠更久，观察父进程是否阻塞
4. 尝试把 `waitpid()` 去掉，再思考会出现什么问题

### 第三阶段：进程间通信

学习重点：

1. `pipe()` 创建的读端和写端是什么
2. 为什么父子进程要关闭不使用的一端
3. 管道适合什么通信场景

练习方法：

1. 运行 `pipe_demo`
2. 让子进程写多条消息
3. 修改成父进程写、子进程读
4. 故意不关闭某一端，观察程序行为

### 第四阶段：线程基础

学习重点：

1. 线程和进程的核心区别
2. `std::thread` 的创建与 `join()`
3. 线程共享进程内存空间

练习方法：

1. 运行 `thread_demo`
2. 增加线程数量
3. 让多个线程共同修改一个变量
4. 对比进程示例和线程示例里的变量行为差异

### 第五阶段：互斥与竞争

学习重点：

1. 什么是数据竞争
2. 为什么 `count++` 不是线程安全的
3. `std::mutex` 和 `std::lock_guard` 的作用

练习方法：

1. 先运行不加锁版本
2. 再运行加锁版本
3. 比较最终计数值
4. 调整线程数和循环次数，加深直觉

### 第六阶段：线程协作

学习重点：

1. 生产者消费者模型
2. `std::condition_variable` 的使用场景
3. 为什么等待条件时要配合锁和谓词

练习方法：

1. 运行 `producer_consumer`
2. 把生产者数量改成 2 个
3. 把消费者数量改成 2 个
4. 尝试限制队列长度，做一个有界缓冲区

## 构建方法

```bash
cd proc_thread
mkdir -p build
cd build
cmake ..
make
```

## 运行方法

```bash
./build/fork_demo
./build/wait_demo
./build/pipe_demo
./build/thread_demo
./build/counter_demo unsafe
./build/counter_demo safe
./build/producer_consumer
```

## 每个示例重点看什么

### `fork_demo`

看父子进程打印的 `pid`、`ppid`、变量值。

### `wait_demo`

看父进程如何通过 `waitpid()` 获得子进程退出状态。

### `pipe_demo`

看管道两端关闭逻辑，以及数据如何从子进程流到父进程。

### `thread_demo`

看线程函数执行、主线程等待、共享变量变化。

### `counter_demo`

看不加锁和加锁的结果差别。

### `producer_consumer`

看多个线程如何围绕共享队列协作。

## 建议学习节奏

1. 每天只学 1 个主题
2. 每个程序至少运行 3 次
3. 每次运行前先猜结果，再执行验证
4. 每学完一个主题，用自己的话写出“为什么”

## 进阶建议

学完这套以后，建议继续往下学：

1. `exec` 系列函数
2. 守护进程
3. 信号 `signal` / `sigaction`
4. 共享内存、消息队列、信号量
5. 线程池
6. epoll 和高并发模型

## 常用文档

1. `man 2 fork`
2. `man 2 waitpid`
3. `man 2 pipe`
4. `man 7 pipe`
5. `man 7 pthreads`
