/**
 * @file StudentManager.h
 * @brief 学生管理类声明
 *
 * 负责学生信息的增删改查，以及使用Linux系统调用进行文件I/O操作
 */

#ifndef STUDENT_MANAGER_H
#define STUDENT_MANAGER_H

#include "Student.h"

 /**
  * @class StudentManager
  * @brief 学生管理类
  *
  * 使用Linux系统调用（open/read/write/close）进行文件操作
  * 管理学生集合，提供增删改查功能
  */
class StudentManager {
private:
    std::string file_path;   //数据文件路径
    Student* students;    // 学生数组
    int count = 0;            // 当前学生数量
    int capacity = 10;         // 数组容量

    /**
     * @brief 从文件加载数据
     * @return 成功返回true，失败返回false
     *
     * 使用open()和read()系统调用读取二进制文件
     */
    bool loadFromFile();

    /**
     * @brief 保存数据到文件
     * @return 成功返回true，失败返回false
     *
     * 使用open()和write()系统调用写入二进制文件
     */
    bool saveToFile();

    /**
     * @brief 扩展数组容量
     */
    void expandCapacity();

public:
    /**
     * @brief 构造函数
     * @param file_path 数据文件路径
     */
    StudentManager(const std::string& file_path);

    /**
     * @brief 析构函数
     */
    ~StudentManager();

    // 禁止拷贝构造和赋值
    StudentManager(const StudentManager&) = delete;
    StudentManager& operator=(const StudentManager&) = delete;

    /**
     * @brief 添加学生
     * @param student 学生对象
     * @return 成功返回true，失败返回false
     */
    bool addStudent(const Student& student);

    /**
     * @brief 删除学生
     * @param id 学号
     * @return 成功返回true，失败返回false
     */
    bool deleteStudent(int id);

    /**
     * @brief 查找学生
     * @param id 学号
     * @return 找到返回学生指针，未找到返回nullptr
     */
    Student* findStudent(int id);

    /**
     * @brief 更新学生信息
     * @param id 学号
     * @param name 新姓名（可选，传nullptr保持不变）
     * @param age 新年龄（可选，传-1保持不变）
     * @param score 新成绩（可选，传-1保持不变）
     * @return 成功返回true，失败返回false
     */
    bool updateStudent(int id, const char* name, int age, float score);

    /**
     * @brief 列出所有学生
     */
    void listAllStudents() const;

    /**
     * @brief 获取学生数量
     * @return 学生数量
     */
    int getCount() const;
};

#endif // STUDENT_MANAGER_H
