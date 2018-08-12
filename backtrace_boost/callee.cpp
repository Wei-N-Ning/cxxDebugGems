
/*
 * Don't forget to modify this def if the backend changes from
 * addr2line to backtrace
 */
//#define BOOST_STACKTRACE_USE_BACKTRACE 1
#define BOOST_STACKTRACE_USE_ADDR2LINE 1

#include <boost/stacktrace.hpp>

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <utility>

using namespace std;
using GroupType = vector<pair<string, string>>;
using DictType = map<int, GroupType>;

template<typename T>
struct Status {
    T m_flag = 0;
    explicit Status(const T& i_value) : m_flag(i_value) {}
};

inline void do_backtrace() {
    std::cout << boost::stacktrace::stacktrace();
}

Status<int> generator(int& numElements, DictType& o_dict) {
    if (! numElements) {
        return Status<int>(0);
    }
    std::vector<int> elements(numElements--, 0);
    std::generate(
        elements.begin(), 
        elements.end(), 
        [&]() -> int {
            return static_cast<int>(elements.size()) + 1;
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
                do_backtrace();
                group.emplace_back(pair<string, string>{"code", "dsm"});
            }
        });
    generator(numElements, o_dict);
    return Status<int>(1);
}

void sut() {
    DictType dict;
    int iterations = 1;
    Status<int> status = generator(iterations, dict);
    if (! status.m_flag) {
        exit(1);
    }
}

