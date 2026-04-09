#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

int main() {
    int localValue = 10;

    std::cout << "fork before: pid=" << getpid()
              << ", localValue=" << localValue << std::endl;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        localValue += 100;
        std::cout << "child process: pid=" << getpid()
                  << ", ppid=" << getppid()
                  << ", localValue=" << localValue << std::endl;
        return 0;
    }

    localValue += 1;
    std::cout << "parent process: pid=" << getpid()
              << ", child pid=" << pid
              << ", localValue=" << localValue << std::endl;

    waitpid(pid, nullptr, 0);
    return 0;
}
