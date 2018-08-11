
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <utility>

#include <execinfo.h>

using namespace std;
using GroupType = vector<pair<string, string>>;
using DictType = map<int, GroupType>;

template<typename T>
struct Status {
    T m_flag = 0;
    Status(const T& i_value) : m_flag(i_value) {}
};

void do_backtrace(int trigger) {
    constexpr int maxSize = 99;
    if (trigger > 2) {
        return;
    }
    vector<void *> addresses(maxSize, nullptr);
    int size = backtrace(addresses.data(), maxSize);
    char** symbols = backtrace_symbols(addresses.data(), size);
    if (symbols != nullptr) {
        for (int i = 0; i < size; ++i)
        {
            cout << symbols[i] << endl;
        }
        free(symbols);
    }

}

Status<int> generator(int& numElements, DictType& o_dict) {
    if (! numElements) {
        return false;
    }
    std::vector<int> elements(numElements--);
    std::generate(
        elements.begin(), 
        elements.end(), 
        [&]() -> int {
            return elements.size() + 1;
        }
    );
    for_each(
        elements.begin(), 
        elements.end(),
        [&](int& elem) {
            GroupType group;
            for (int i = 0; i < elem + 1; ++i)
            {
                group.emplace_back(pair<string, string>{"phrase", "x12"});
                do_backtrace(numElements);
                group.emplace_back(pair<string, string>{"code", "dsm"});
            }
        });
    generator(numElements, o_dict);
    return true;
}

void sut() {
    DictType dict;
    int iterations = 4;
    Status<int> status = generator(iterations, dict);
    if (! status.m_flag) {
        exit(1);
    }
}

