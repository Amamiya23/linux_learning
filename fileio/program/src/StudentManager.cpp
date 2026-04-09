#include "StudentManager.h"
#include <iostream>
#include <cstring>

// Linux系统调用头文件
#include <fcntl.h>      // open() 相关标志
#include <unistd.h>     // read(), write(), close()
#include <errno.h>      // errno 错误码
#include <cstring>      // strerror()

/**
 * @brief 构造函数
 *
 * 初始化管理器，尝试从文件加载已有数据
 */
StudentManager::StudentManager(const std::string& file_path){

    // 分配学生数组内存
    students = new Student[capacity];
    // 尝试从文件加载数据
    loadFromFile();
}

/**
 * @brief 析构函数
 *
 * 释放内存，确保数据已保存
 */
StudentManager::~StudentManager() {
    // 保存数据到文件
    saveToFile();

    // 释放内存
    delete[] students;
}

/**
 * @brief 从文件加载数据
 *
 * 学习要点：
 * 1. open() 系统调用的使用
 * 2. read() 系统调用读取二进制数据
 * 3. 错误处理：errno 和 perror
 *
 * @return 成功返回true，失败返回false
 */
bool StudentManager::loadFromFile() {
    // 使用 open() 系统调用打开文件
    // O_RDONLY: 只读模式
    // 如果文件不存在，open() 返回 -1，设置 errno
    int fd = open(file_path.c_str(), O_RDONLY);

    if (fd == -1) {
        // 文件不存在是正常情况（首次运行），不报错
        if (errno == ENOENT) {
            std::cout << "数据文件不存在，将创建新文件: " << file_path << std::endl;
            return true;
        }
        // 其他错误
        perror("打开文件失败");
        return false;
    }

    std::cout << "正在从文件加载数据..." << std::endl;

    // 读取学生记录
    Student tempStudent;
    ssize_t bytesRead;

    // 使用 read() 系统调用循环读取每条记录
    // read() 返回实际读取的字节数，0表示文件结束，-1表示错误
    while ((bytesRead = read(fd, &tempStudent, sizeof(Student))) > 0) {
        // 检查是否读取了完整的记录
        if (bytesRead != sizeof(Student)) {
            std::cerr << "读取到不完整的记录" << std::endl;
            break;
        }

        // 检查是否需要扩容
        if (count >= capacity) {
            expandCapacity();
        }

        // 添加到数组
        students[count++] = tempStudent;
    }

    // 检查读取错误
    if (bytesRead == -1) {
        perror("读取文件失败");
        close(fd);  
        return false;
    }

    // 使用 close() 系统调用关闭文件描述符
    close(fd);

    std::cout << "成功加载 " << count << " 条学生记录" << std::endl;
    return true;
}

/**
 * @brief 保存数据到文件
 *
 * @return 成功返回true，失败返回false
 */
bool StudentManager::saveToFile() {
    // 使用 open() 系统调用创建/截断文件
    // O_WRONLY: 只写模式
    // O_CREAT: 文件不存在则创建
    // O_TRUNC: 如果文件存在，截断（清空）文件内容
    // 0644: 文件权限 (rw-r--r--)
    int fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("创建/打开文件失败");
        return false;
    }

    // 使用 write() 系统调用写入每条学生记录
    for (int i = 0; i < count; i++) {
        ssize_t bytesWritten = write(fd, &students[i], sizeof(Student));

        // 检查写入是否成功
        if (bytesWritten == -1) {
            perror("写入文件失败");
            close(fd);
            return false;
        }

        // 检查是否写入了完整的记录
        if (bytesWritten != sizeof(Student)) {
            std::cerr << "错误: 写入不完整" << std::endl;
            close(fd);
            return false;
        }
    }

    //关闭文件描述符
    close(fd);

    std::cout << "数据已保存到文件: " << file_path<< std::endl;
    return true;
}

/**
 * @brief 扩展数组容量
 *
 * 当数组满时，分配更大的内存空间
 */
void StudentManager::expandCapacity() {
    int newCapacity = capacity * 2;
    Student* newStudents = new Student[newCapacity];

    // 复制旧数据
    for (int i = 0; i < count; i++) {
        newStudents[i] = students[i];
    }

    // 释放旧内存
    delete[] students;

    // 更新指针和容量
    students = newStudents;
    capacity = newCapacity;

    std::cout << "数组已扩容至 " << capacity << " 个位置" << std::endl;
}

/**
 * @brief 添加学生
 */
bool StudentManager::addStudent(const Student& student) {
    // 检查学号是否已存在
    if (findStudent(student.id) != nullptr) {
        std::cerr << "错误: 学号 " << student.id << " 已存在" << std::endl;
        return false;
    }

    // 检查是否需要扩容
    if (count >= capacity) {
        expandCapacity();
    }

    // 添加学生
    students[count++] = student;

    // 保存到文件
    return saveToFile();
}

/**
 * @brief 删除学生
 */
bool StudentManager::deleteStudent(int id) {
    int index = -1;

    // 查找学生位置
    for (int i = 0; i < count; i++) {
        if (students[i].id == id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        std::cerr << "未找到学号为 " << id << " 的学生" << std::endl;
        return false;
    }

    // 移动后面的元素覆盖要删除的元素
    for (int i = index; i < count - 1; i++) {
        students[i] = students[i + 1];
    }

    count--;

    // 保存到文件
    return saveToFile();
}

/**
 * @brief 查找学生
 */
Student* StudentManager::findStudent(int id) {
    for (int i = 0; i < count; i++) {
        if (students[i].id == id) {
            return &students[i];
        }
    }
    return nullptr;
}

/**
 * @brief 更新学生信息
 * 更新结构体数组，把结构体数组重新在文件写一遍
 */
bool StudentManager::updateStudent(int id, const char* name, int age, float score) {
    Student* student = findStudent(id);

    if (student == nullptr) {
        std::cerr << "未找到学号为 " << id << " 的学生" << std::endl;
        return false;
    }

    // 更新信息（只更新非默认值的字段）
    if (name != nullptr) {
        strncpy(student->name, name, sizeof(student->name) - 1);
        student->name[sizeof(student->name) - 1] = '\0';
    }
    if (age >= 0) {
        student->age = age;
    }
    if (score >= 0) {
        student->score = score;
    }

    // 保存到文件
    return saveToFile();
}

/**
 * @brief 列出所有学生
 */
void StudentManager::listAllStudents() const {
    if (count == 0) {
        std::cout << "暂无学生记录" << std::endl;
        return;
    }

    std::cout << "\n========== 学生列表 ==========" << std::endl;
    std::cout << "共 " << count << " 条记录" << std::endl;
    std::cout << "------------------------------" << std::endl;

    for (int i = 0; i < count; i++) {
        students[i].display();
    }

    std::cout << "==============================" << std::endl;
}

/**
 * @brief 获取学生数量
 */
int StudentManager::getCount() const {
    return count;
}
