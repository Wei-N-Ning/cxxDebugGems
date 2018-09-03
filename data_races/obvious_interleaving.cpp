//
// Created by wein on 3/09/18.
//
/*

 ==================
WARNING: ThreadSanitizer: data race (pid=14138)
  Write of size 4 at 0x7fffa32ab0a0 by thread T2:
    #0 Accessor::set(int) <null> (obvious_interleaving+0x220c)
    #1 main::{lambda(int)#1}::operator()(int) const <null> (obvious_interleaving+0x13e9)
    #2 int std::__invoke_impl<int, main::{lambda(int)#1}, int>(std::__invoke_other, main::{lambda(int)#1}&&, int&&) <null> (obvious_interleaving+0x19c3)
    #3 std::__invoke_result<main::{lambda(int)#1}, int>::type std::__invoke<main::{lambda(int)#1}, int>(std::__invoke_result&&, (main::{lambda(int)#1}&&)...) <null> (obvious_interleaving+0x16b5)
    #4 decltype (__invoke((_S_declval<0ul>)(), (_S_declval<1ul>)())) std::thread::_Invoker<std::tuple<main::{lambda(int)#1}, int> >::_M_invoke<0ul, 1ul>(std::_Index_tuple<0ul, 1ul>) <null> (obvious_interleaving+0x1e92)
    #5 std::thread::_Invoker<std::tuple<main::{lambda(int)#1}, int> >::operator()() <null> (obvious_interleaving+0x1e06)
    #6 std::thread::_State_impl<std::thread::_Invoker<std::tuple<main::{lambda(int)#1}, int> > >::_M_run() <null> (obvious_interleaving+0x1dac)
    #7 <null> <null> (libstdc++.so.6+0xbe732)

  Previous write of size 4 at 0x7fffa32ab0a0 by thread T1:
    #0 Accessor::set(int) <null> (obvious_interleaving+0x220c)
    #1 main::{lambda(int)#1}::operator()(int) const <null> (obvious_interleaving+0x13e9)
    #2 int std::__invoke_impl<int, main::{lambda(int)#1}, int>(std::__invoke_other, main::{lambda(int)#1}&&, int&&) <null> (obvious_interleaving+0x19c3)
    #3 std::__invoke_result<main::{lambda(int)#1}, int>::type std::__invoke<main::{lambda(int)#1}, int>(std::__invoke_result&&, (main::{lambda(int)#1}&&)...) <null> (obvious_interleaving+0x16b5)
    #4 decltype (__invoke((_S_declval<0ul>)(), (_S_declval<1ul>)())) std::thread::_Invoker<std::tuple<main::{lambda(int)#1}, int> >::_M_invoke<0ul, 1ul>(std::_Index_tuple<0ul, 1ul>) <null> (obvious_interleaving+0x1e92)
    #5 std::thread::_Invoker<std::tuple<main::{lambda(int)#1}, int> >::operator()() <null> (obvious_interleaving+0x1e06)
    #6 std::thread::_State_impl<std::thread::_Invoker<std::tuple<main::{lambda(int)#1}, int> > >::_M_run() <null> (obvious_interleaving+0x1dac)
    #7 <null> <null> (libstdc++.so.6+0xbe732)

  Location is stack of main thread.

  Location is global '<null>' at 0x000000000000 ([stack]+0x00000001f0a0)

  Thread T2 (tid=14141, running) created by main thread at:
    #0 pthread_create <null> (libtsan.so.0+0x2bec2)
    #1 std::thread::_M_start_thread(std::unique_ptr<std::thread::_State, std::default_delete<std::thread::_State> >, void (*)()) <null> (libstdc++.so.6+0xbea18)
    #2 main <null> (obvious_interleaving+0x14ef)

  Thread T1 (tid=14140, finished) created by main thread at:
    #0 pthread_create <null> (libtsan.so.0+0x2bec2)
    #1 std::thread::_M_start_thread(std::unique_ptr<std::thread::_State, std::default_delete<std::thread::_State> >, void (*)()) <null> (libstdc++.so.6+0xbea18)
    #2 main <null> (obvious_interleaving+0x14ef)

SUMMARY: ThreadSanitizer: data race (/work/dev/cxx/github.com/powergun/cxxDebugGems/build/data_races/obvious_interleaving+0x220c) in Accessor::set(int)
==================


 */
#include <algorithm>
#include <thread>
#include <cassert>
#include <iostream>
#include <vector>

struct Accessor {
    explicit Accessor(int* data)
        :m_data(data) {
    }

    void set(int v) {
        *m_data = v;
    }

    int get() {
        return *m_data;
    }

    int* m_data = nullptr;
};

int main() {
    int a = 0xDEAD;

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