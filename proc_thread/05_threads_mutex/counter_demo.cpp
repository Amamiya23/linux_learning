#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr int kThreadCount = 4;
constexpr int kLoopCount = 100000;

void unsafeIncrement(int &counter) {
    for (int i = 0; i < kLoopCount; ++i) {
        ++counter;
    }
}

void safeIncrement(int &counter, std::mutex &mutex) {
    for (int i = 0; i < kLoopCount; ++i) {
        std::lock_guard<std::mutex> lock(mutex);
        ++counter;
    }
}

}  // namespace

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " [unsafe|safe]" << std::endl;
        return EXIT_FAILURE;
    }

    std::string mode = argv[1];
    int counter = 0;
    std::vector<std::thread> threads;
    std::mutex mutex;

    if (mode == "unsafe") {
        for (int i = 0; i < kThreadCount; ++i) {
            threads.emplace_back(unsafeIncrement, std::ref(counter));
        }
    } else if (mode == "safe") {
        for (int i = 0; i < kThreadCount; ++i) {
            threads.emplace_back(safeIncrement, std::ref(counter), std::ref(mutex));
        }
    } else {
        std::cerr << "invalid mode: " << mode << std::endl;
        return EXIT_FAILURE;
    }

    for (auto &thread : threads) {
        thread.join();
    }

    std::cout << "mode=" << mode << std::endl;
    std::cout << "expected=" << kThreadCount * kLoopCount << std::endl;
    std::cout << "actual=" << counter << std::endl;
    return 0;
}
