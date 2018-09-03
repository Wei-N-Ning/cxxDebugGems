//
// Created by wein on 3/09/18.
//

#include <algorithm>
#include <thread>
#include <cassert>
#include <iostream>
#include <vector>
#include <atomic>

// a cheap method to "fix" the data race:
// apply only the atomic operations on the mutable data
// but the design is still wrong

// NOTE acc is still concurrently read/written by multiple
// threads but setter getter now are atomic

// here is a good read:
// https://stackoverflow.com/questions/30549327/best-way-to-print-information-when-debugging-a-race-condition
// why GDB would be useless in this case

// No, it's much worse. Every breakpoint that is hit in GDB triggers the following chain of events:
//context switch from running thread to GDB
//GDB stops all other threads (assuming default all-stop mode)
//GDB evaluates breakpoint commands
//GDB resumes all threads (this is itself a complicated multi-step process, which I would not go into here).
// I second the ThreadSanitizer recommendation by Christopher Ian Stern.


struct Accessor {
    explicit Accessor(std::atomic<int>* data)
        :m_data(data) {
    }

    void set(int v) {
        *m_data = v;
    }

    int get() {
        return *m_data;
    }

    std::atomic<int>* m_data = nullptr;
};

int main() {
    std::atomic<int> a{0xDEAD};
    auto worker = [&](int v) -> int {
        Accessor acc(&a);
        for (int i = 1000 - v * 123; i--; ) {
        }
        acc.set(v);
        for (int i = 1000 - v * 231; i--; ) {
        }
        int expected = acc.get();
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

    std::cout << a << std::endl;

    return 0;
}