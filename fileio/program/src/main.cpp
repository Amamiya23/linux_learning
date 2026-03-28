#include "StudentManager.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

 // 数据文件路径
const char* DATA_FILE = "data/students.dat";

void showHelp(const char* programName) {
    std::cout << "\n========== 学生信息管理系统 ==========\n" << std::endl;
    std::cout << "使用方法:" << std::endl;
    std::cout << "  " << programName << " <命令> [选项]\n" << std::endl;

    std::cout << "命令:" << std::endl;
    std::cout << "  add     添加学生" << std::endl;
    std::cout << "  delete  删除学生" << std::endl;
    std::cout << "  find    查找学生" << std::endl;
    std::cout << "  update  更新学生信息" << std::endl;
    std::cout << "  list    列出所有学生\n" << std::endl;

    std::cout << "选项:" << std::endl;
    std::cout << "  --id <学号>      学号（整数）" << std::endl;
    std::cout << "  --name <姓名>    姓名（字符串）" << std::endl;
    std::cout << "  --age <年龄>     年龄（整数）" << std::endl;
    std::cout << "  --score <成绩>   成绩（浮点数）\n" << std::endl;

    std::cout << "示例:" << std::endl;
    std::cout << "  " << programName << " add --id 1 --name \"张三\" --age 20 --score 85.5" << std::endl;
    std::cout << "  " << programName << " delete --id 1" << std::endl;
    std::cout << "  " << programName << " find --id 1" << std::endl;
    std::cout << "  " << programName << " update --id 1 --name \"李四\" --age 21" << std::endl;
    std::cout << "  " << programName << " list\n" << std::endl;

    std::cout << "======================================\n" << std::endl;
}


bool parseOption(char** argv, int argc, const char* option, const char** value) {
    for (int i = 0; i < argc - 1; i++) {
        if (strcmp(argv[i], option) == 0) {
            *value = argv[i + 1];
            return true;
        }
    }
    return false;
}

int handleAdd(StudentManager& manager, char** argv, int argc) {
    const char* idStr = nullptr;
    const char* name = nullptr;
    const char* ageStr = nullptr;
    const char* scoreStr = nullptr;

    if (!parseOption(argv, argc, "--id", &idStr) ||
        !parseOption(argv, argc, "--name", &name) ||
        !parseOption(argv, argc, "--age", &ageStr) ||
        !parseOption(argv, argc, "--score", &scoreStr)) {
        std::cerr << "错误: 添加学生需要提供所有参数" << std::endl;
        std::cerr << "用法: add --id <学号> --name <姓名> --age <年龄> --score <成绩>" << std::endl;
        return 1;
    }

    int id = atoi(idStr);
    int age = atoi(ageStr);
    float score = atof(scoreStr);

    Student student(id, name, age, score);

    if (manager.addStudent(student)) {
        std::cout << "成功添加学生: " << name << std::endl;
        return 0;
    }

    return 1;
}


int handleDelete(StudentManager& manager, char** argv, int argc) {
    const char* idStr = nullptr;

    if (!parseOption(argv, argc, "--id", &idStr)) {
        std::cerr << "错误: 需要指定学号" << std::endl;
        std::cerr << "用法: delete --id <学号>" << std::endl;
        return 1;
    }

    int id = atoi(idStr);

    if (manager.deleteStudent(id)) {
        std::cout << "成功删除学号为 " << id << " 的学生" << std::endl;
        return 0;
    }

    return 1;
}

/**
 * @brief 查找学生命令处理
 */
int handleFind(StudentManager& manager, char** argv, int argc) {
    const char* idStr = nullptr;

    if (!parseOption(argv, argc, "--id", &idStr)) {
        std::cerr << "错误: 需要指定学号" << std::endl;
        std::cerr << "用法: find --id <学号>" << std::endl;
        return 1;
    }

    int id = atoi(idStr);
    Student* student = manager.findStudent(id);

    if (student != nullptr) {
        std::cout << "\n找到学生:" << std::endl;
        std::cout << "------------------------------" << std::endl;
        student->display();
        std::cout << "------------------------------\n" << std::endl;
        return 0;
    }

    std::cerr << "未找到学号为 " << id << " 的学生" << std::endl;
    return 1;
}

/**
 * @brief 更新学生命令处理
 */
int handleUpdate(StudentManager& manager, char** argv, int argc) {
    const char* idStr = nullptr;
    const char* name = nullptr;
    const char* ageStr = nullptr;
    const char* scoreStr = nullptr;

    if (!parseOption(argv, argc, "--id", &idStr)) {
        std::cerr << "错误: 需要指定学号" << std::endl;
        std::cerr << "用法: update --id <学号> [--name <姓名>] [--age <年龄>] [--score <成绩>]" << std::endl;
        return 1;
    }

    int id = atoi(idStr);

    // 可选参数
    parseOption(argv, argc, "--name", &name);
    parseOption(argv, argc, "--age", &ageStr);
    parseOption(argv, argc, "--score", &scoreStr);

    int age = ageStr ? atoi(ageStr) : -1;
    float score = scoreStr ? atof(scoreStr) : -1.0f;

    if (manager.updateStudent(id, name, age, score)) {
        std::cout << "成功更新学号为 " << id << " 的学生信息" << std::endl;
        return 0;
    }

    return 1;
}

/**
 * @brief 列出所有学生命令处理
 */
int handleList(StudentManager& manager) {
    manager.listAllStudents();
    return 0;
}

/**
 * @brief 主函数
 */
int main(int argc, char* argv[]) {
    // 检查参数数量
    if (argc < 2) {
        showHelp(argv[0]);
        return 0;
    }

    // 显示帮助
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        showHelp(argv[0]);
        return 0;
    }

    // 创建学生管理器
    StudentManager manager(DATA_FILE);

    // 解析命令
    const char* command = argv[1];

    if (strcmp(command, "add") == 0) {
        return handleAdd(manager, argv + 2, argc - 2);
    }
    else if (strcmp(command, "delete") == 0) {
        return handleDelete(manager, argv + 2, argc - 2);
    }
    else if (strcmp(command, "find") == 0) {
        return handleFind(manager, argv + 2, argc - 2);
    }
    else if (strcmp(command, "update") == 0) {
        return handleUpdate(manager, argv + 2, argc - 2);
    }
    else if (strcmp(command, "list") == 0) {
        return handleList(manager);
    }
    else {
        std::cerr << "未知命令: " << command << std::endl;
        showHelp(argv[0]);
        return 1;
    }
}
