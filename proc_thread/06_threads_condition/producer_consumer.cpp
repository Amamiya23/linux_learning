#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

namespace {

std::queue<int> tasks;
std::mutex mutex;
std::condition_variable condition;
bool finished = false;

void producer() {
    for (int value = 1; value <= 5; ++value) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            tasks.push(value);
            std::cout << "producer: pushed " << value << std::endl;
        }

        condition.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    {
        std::lock_guard<std::mutex> lock(mutex);
        finished = true;
    }
    condition.notify_all();
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [] { return finished || !tasks.empty(); });

        if (tasks.empty() && finished) {
            break;
        }

        int value = tasks.front();
        tasks.pop();
        lock.unlock();

        std::cout << "consumer: got " << value << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

}  // namespace

int main() {
    std::thread producerThread(producer);
    std::thread consumerThread(consumer);

    producerThread.join();
    consumerThread.join();

    std::cout << "all tasks processed" << std::endl;
    return 0;
}
