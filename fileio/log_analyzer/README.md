# Linux 文件I/O 综合练习项目：日志文件分析器

## 项目概述

这是一个综合性的Linux文件I/O学习项目，通过实现一个实用的日志文件分析工具，全面掌握文件操作的核心技能。项目涵盖了从基础文件读写到高级内存映射、目录遍历等关键知识点，具有实际应用价值。

## 项目特色

- **实际应用价值**：可直接用于分析应用程序日志，生成统计报告
- **完整覆盖文件I/O**：涵盖标准I/O、系统调用、内存映射等多种技术
- **模块化设计**：清晰的架构便于学习和扩展
- **多格式支持**：文本、CSV、JSON、HTML等多种输出格式
- **命令行工具**：提供多个实用命令行工具

## 项目结构

```
log_analyzer/
├── CMakeLists.txt          # CMake构建配置（备用）
├── Makefile                # 主构建文件
├── include/                # 处文件目录
│   ├── log_parser.h       # 日志解析器接口
│   ├── file_reader.h      # 文件读取器接口
│   ├── log_filter.h       # 日志过滤器接口
│   ├── report_generator.h # 报告生成器接口
│   └── directory_walker.h # 目录遍历器接口
├── src/                    # 源代码目录
│   ├── main.cpp           # 主程序入口
│   ├── log_parser.cpp     # 日志解析器实现
│   ├── file_reader.cpp    # 文件读取器实现
│   ├── log_filter.cpp     # 日志过滤器实现
│   ├── report_generator.cpp # 报告生成器实现
│   ├── directory_walker.cpp # 目录遍历器实现
│   └── tools/             # 工具程序
│       ├── log_stats.cpp  # 日志统计工具
│       ├── log_search.cpp # 日志搜索工具
│       └── log_format.cpp # 日志格式化工具
├── tests/                  # 测试程序
│   ├── test_parser.cpp    # 解析器测试
│   └── test_file_reader.cpp # 文件读取器测试
├── data/                   # 示例日志文件
│   ├── sample.log         # 标准格式日志
│   ├── app.log            # 应用日志
│   └── json.log           # JSON格式日志
├── bin/                    # 可执行文件目录
├── obj/                    # 编译对象文件目录
└── build/                  # CMake构建目录
```

## 学习要点

### 1. 文件基础操作
- 文件打开、读取、写入、关闭
- 错误处理与`errno`使用
- 文件描述符与FILE指针

### 2. 文件读取模式
- **标准I/O**：使用`ifstream`进行缓冲读取
- **内存映射**：使用`mmap`实现高效文件访问
- 大文件处理策略

### 3. 目录操作
- 目录遍历与文件筛选
- 文件状态获取（`stat`系统调用）
- 递归目录处理

### 4. 日志解析技术
- 正则表达式匹配日志格式
- 时间戳解析与转换
- 多种日志格式支持

### 5. 数据过滤与统计
- 按条件过滤日志条目
- 统计信息收集与分析
- 数据聚合与排序

### 6. 报告生成
- 多格式输出（文本、CSV、JSON、HTML）
- 文件写入与格式化
- 控制台彩色输出

## 构建与运行

### 快速开始
```bash
# 进入项目目录
cd /home/cat/Learning/log_analyzer

# 编译项目
make

# 运行测试
make test

# 分析单个日志文件
./bin/log_analyzer data/sample.log

# 查看统计信息
./bin/log_analyzer -s data/sample.log

# 过滤错误日志
./bin/log_analyzer -l error data/sample.log

# 生成JSON报告
./bin/log_analyzer -f json -o report.json data/sample.log
```

### 完整命令选项
```
用法: log_analyzer [选项] <文件或目录>

选项:
  -f, --format <格式>    输出格式: text, csv, json, html (默认: text)
  -o, --output <文件>    输出文件 (默认: 标准输出)
  -l, --level <级别>     按最低级别过滤: trace, debug, info, warn, error, fatal
  -L, --logger <名称>    按日志器名称过滤
  -m, --message <模式>   按消息模式过滤 (正则表达式)
  -s, --stats            仅显示统计信息
  -r, --recursive        递归处理目录
  -c, --no-color         禁用彩色输出
  -h, --help             显示帮助信息
```

## 工具程序

### log_stats - 日志统计工具
```bash
./bin/log_stats data/sample.log
./bin/log_stats -l error data/  # 只统计错误级别
```

### log_search - 日志搜索工具
```bash
./bin/log_search data/sample.log "database"
./bin/log_search -i data/sample.log "error"  # 忽略大小写
```

### log_format - 日志格式化工具
```bash
./bin/log_format data/sample.log -f csv -o output.csv
./bin/log_format data/sample.log -f html -o report.html
```

## 扩展建议

### 初级扩展
1. 添加更多日志格式支持（如Apache、Nginx日志）
2. 实现时间范围过滤（`--start-time`, `--end-time`）
3. 添加日志级别颜色配置

### 中级扩展
1. 实现文件锁机制（`flock`）处理并发访问
2. 添加多线程处理提高性能
3. 实现实时日志监控（`inotify`）

### 高级扩展
1. 添加日志压缩/解压缩支持
2. 实现日志数据库存储
3. 构建Web界面展示统计信息

## 常见问题

### Q: 如何添加新的日志格式支持？
A: 在`log_parser.cpp`中扩展`use_*_pattern()`函数，添加新的正则表达式模式。

### Q: 如何处理超大日志文件？
A: 项目已实现内存映射（mmap）读取大文件，可通过`FileReaderFactory::create_optimal_reader`自动选择。

### Q: 如何扩展过滤条件？
A: 修改`FilterCriteria`结构体，添加新条件，并在`LogFilter`中实现相应检查逻辑。

## 学习资源

- 《UNIX环境高级编程》- 文件I/O章节
- Linux man pages: `open(2)`, `read(2)`, `write(2)`, `mmap(2)`, `stat(2)`
- C++17文件系统库（可选扩展）

---

**项目状态**：✅ 已完成基础功能，通过所有测试  
**适用场景**：Linux应用开发学习、文件I/O实践、日志处理工具开发