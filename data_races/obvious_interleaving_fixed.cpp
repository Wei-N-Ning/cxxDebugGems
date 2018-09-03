//
// Created by wein on 3/09/18.
//

#include <algorithm>
#include <thread>
#include <cassert>
#include <iostream>
#include <vector>
#include <atomic>

// to fix it the design must be changed

struct Accessor {
    explicit Accessor(std::atomic<int>* data)
        :m_data(data) {
    }

    void incr() {
        (*m_data)++;
    }

    std::atomic<int>* m_data = nullptr;
};

int main() {
    std::atomic<int> a{0};
    auto worker = [&](int v) -> int {
        Accessor acc(&a);
        for (int i = 1000 - v * 123; i--; ) {
        }
        acc.incr();
        for (int i = 1000 - v * 231; i--; ) {
        }
        return v;
    };

    constexpr int numThreads = 8;
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::move(std::thread(worker, i)));
    }

    for (auto& th : threads) {
        th.join();
    }

    // the result is 8 because there are 8 workers;
    // each worker increments the mutable data by 1
    std::cout << a << std::endl;

    return 0;
}