# 学生信息管理系统

一个用于学习Linux文件I/O系统调用的学生信息管理系统。采用C++面向对象设计，使用Linux系统调用进行文件操作。

## 学习目标

通过本项目，你将学习到：

1. **Linux系统调用**
   - `open()` - 打开/创建文件
   - `read()` - 读取文件内容
   - `write()` - 写入文件内容
   - `close()` - 关闭文件描述符

2. **文件操作概念**
   - 文件描述符（File Descriptor）
   - 文件权限（如 0644）
   - 打开标志（O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC, O_APPEND）

3. **错误处理**
   - `errno` - 错误码
   - `perror()` - 错误信息输出

4. **面向对象设计**
   - 封装
   - 单一职责原则

## 项目结构

```
program/
├── include/              # 头文件目录
│   ├── Student.h         # 学生类声明
│   └── StudentManager.h  # 管理类声明
├── src/                  # 源文件目录
│   ├── Student.cpp       # 学生类实现
│   ├── StudentManager.cpp# 管理类实现
│   └── main.cpp          # 主程序
├── data/                 # 数据目录
│   └── students.dat      # 学生数据文件（运行时生成）
├── CMakeLists.txt        # CMake构建脚本
└── README.md             # 项目说明
```

## 构建方法

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make

# 返回项目根目录
cd ..
```

## 使用方法

```bash
# 显示帮助
./build/student_sys --help

# 添加学生
./build/student_sys add --id 1 --name "张三" --age 20 --score 85.5
./build/student_sys add --id 2 --name "李四" --age 21 --score 90.0
./build/student_sys add --id 3 --name "王五" --age 19 --score 78.5

# 列出所有学生
./build/student_sys list

# 查找学生
./build/student_sys find --id 1

# 更新学生信息
./build/student_sys update --id 1 --name "张三丰" --age 22

# 删除学生
./build/student_sys delete --id 2
```

## 核心代码解析

### 文件打开 - open()

```cpp
#include <fcntl.h>

// 打开已存在的文件（只读）
int fd = open("students.dat", O_RDONLY);

// 创建新文件（权限：0644）
int fd = open("students.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
```

### 文件读取 - read()

```cpp
#include <unistd.h>

Student student;
ssize_t bytesRead = read(fd, &student, sizeof(Student));
if (bytesRead == -1) {
    perror("读取失败");
}
```

### 文件写入 - write()

```cpp
#include <unistd.h>

ssize_t bytesWritten = write(fd, &student, sizeof(Student));
if (bytesWritten == -1) {
    perror("写入失败");
}
```

### 文件关闭 - close()

```cpp
#include <unistd.h>

close(fd);
```

## 数据存储格式

采用二进制格式存储，每个学生记录固定大小：

```
+--------+------------------+--------+--------+
|   id   |      name        |  age   | score  |
| 4 bytes|    50 bytes      | 4 bytes| 4 bytes|
+--------+------------------+--------+--------+
总大小：62 bytes/记录
```

## 扩展学习

完成基础功能后，可以尝试：

- 使用 `lseek()` 实现随机访问修改
- 添加文件锁 `flock()` 防止并发问题
- 使用 `stat()` 获取文件信息
- 实现数据导入导出功能

## 参考资料

- [Linux Programmer's Manual](https://man7.org/linux/man-pages/)
- `man 2 open`
- `man 2 read`
- `man 2 write`
- `man 2 close`
