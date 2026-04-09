#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

int main() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        std::cout << "child: start working, pid=" << getpid() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "child: finish work, exit code = 42" << std::endl;
        return 42;
    }

    std::cout << "parent: waiting child pid=" << pid << std::endl;

    int status = 0;
    pid_t waitedPid = waitpid(pid, &status, 0);
    if (waitedPid < 0) {
        perror("waitpid failed");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        std::cout << "parent: child exited normally, code="
                  << WEXITSTATUS(status) << std::endl;
    } else if (WIFSIGNALED(status)) {
        std::cout << "parent: child killed by signal="
                  << WTERMSIG(status) << std::endl;
    }

    return 0;
}
