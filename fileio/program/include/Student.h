/**
 * @file Student.h
 * @brief 学生结构体定义
 *
 * 使用struct定义学生数据结构，简洁明了
 * 便于二进制文件读写
 */

#ifndef STUDENT_H
#define STUDENT_H

#include <iostream>
#include <iomanip>
#include <cstring>

 /**
  * @struct Student
  * @brief 学生数据结构
  *
  * 使用固定大小的字符数组存储姓名，便于二进制文件读写。
  * 所有成员默认public，直接访问
  */
struct Student {
    int id;           // 学号
    char name[50];    // 姓名（固定50字节）
    int age;          // 年龄
    float score;      // 成绩

    /**
     * @brief 无参构造函数
     * 
     * memset将 name 数组的所有字节清零。这确保了字符数组以空字符 \0 开头
     */
    Student() : id(0), age(0), score(0.0f) {
        memset(name, 0, sizeof(name));
    }

    /**
     * @brief 参数构造函数
     */
    Student(int id, const char* name, int age, float score): id(id), age(age), score(score) {
        strncpy(this->name, name, sizeof(this->name) - 1);
        this->name[sizeof(this->name) - 1] = '\0';
    }

    /**
     * @brief 显示学生信息
     */
    void display() const {
        std::cout << "学号: " << std::setw(6) << id
            << " | 姓名: " << std::setw(15) << name
            << " | 年龄: " << std::setw(3) << age
            << " | 成绩: " << score
            << std::endl;
    }
};

#endif // STUDENT_H
