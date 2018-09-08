
#include <cassert>

// source
// exceptional P131
// this chapter talks about class design and inheritance but I find it 
// a good debugging/hacking technique too
//

// P141
// any member template can be specialized for any type. 
// specialize it on a type that you know nody else will ever specialize 
// it on (say, a type in your own unnamed namespace)


// sut header sut.h
class SUT {
public:
    SUT(int v) : m_value(v) {}
    template<class T>
    void f(const T& t) {}
    int value() { return m_value; }
private:
    int m_value = 0;
};

void RunTinyTests();

// 1) dup definition
// instead of including sut.h, manually (and illegally) duplicate sut's 
// definition, and adds a line such as:
// NOTE both definition can not appear in the same translation unit
// otherwise compilation fails
class SUT_ {
public:
    SUT_(int v) : m_value(v) {}
    template<class T>
    void f(const T& t) {}
    int value() { return m_value; }
    int m_value = 0;
};

void test_dup_sut() {
    SUT_ s(1);
    s.m_value = 1337;
}

// 2) macro private public
#define private public

// 3) reinterpret_cast<> and class layout abusing
class SUT__ {
public:
    int m_value = 0;
};

void test_reinterpret_cast_hack() {
    SUT sut(10);
    reinterpret_cast<SUT__&>(sut).m_value = 1337;
    assert(1337 == sut.value());
}

// 4) abuse the member function template and inject a doer 
struct Doer {
    void modify(int& value) const {
        value = 1337;
    }
};

template<>
void SUT::f(const Doer& doer) {
    doer.modify(m_value);
}

void test_member_function_template_exploit() {
    SUT sut(10);
    sut.f(Doer());
    assert(1337 == sut.value());
}

int main() {
    RunTinyTests();
    return 0;
}
