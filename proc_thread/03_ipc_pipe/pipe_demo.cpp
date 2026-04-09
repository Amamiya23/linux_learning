#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

int main() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        close(pipefd[0]);

        std::string message = "hello from child process";
        ssize_t written = write(pipefd[1], message.c_str(), message.size());
        if (written == -1) {
            perror("child write failed");
            close(pipefd[1]);
            return EXIT_FAILURE;
        }

        std::cout << "child: wrote " << written << " bytes" << std::endl;
        close(pipefd[1]);
        return 0;
    }

    close(pipefd[1]);

    char buffer[128] = {0};
    ssize_t bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1);
    if (bytesRead == -1) {
        perror("parent read failed");
        close(pipefd[0]);
        waitpid(pid, nullptr, 0);
        return EXIT_FAILURE;
    }

    std::cout << "parent: received message = \"" << buffer << "\"" << std::endl;

    close(pipefd[0]);
    waitpid(pid, nullptr, 0);
    return 0;
}
