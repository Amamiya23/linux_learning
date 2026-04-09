#include <unistd.h>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

void worker(int index, int &sharedValue) {
    for (int i = 0; i < 3; ++i) {
        ++sharedValue;
        std::cout << "thread " << index
                  << " in process " << getpid()
                  << ", sharedValue=" << sharedValue << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    int sharedValue = 0;
    std::vector<std::thread> threads;

    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(worker, i, std::ref(sharedValue));
    }

    for (auto &thread : threads) {
        thread.join();
    }

    std::cout << "main thread: final sharedValue=" << sharedValue << std::endl;
    return 0;
}
