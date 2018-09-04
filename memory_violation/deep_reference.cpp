//
// Created by wein on 5/09/18.
//

// note
// this example is inspired by real event at wt
// a complex computing system implements a high level memory management
// component that abstracts away heap memory
// it offers two concepts:
// data: knows where to look for resources in heap
// reference: bears the knowledge of data; it acts like a pointer to the
// data
// data can have children so does reference
// a reference to a tree of data is also a tree, with each (reference)
// node pointing to its data node counterpart

#include <vector>

struct FData {
    std::vector<FData> m_elements;
    int* m_resource = nullptr;

    ~FData() {
        delete m_resource;
    }
};

struct FRef {
    std::vector<FRef> m_elements;
    FData* m_data = nullptr;

    FRef& operator= (int val) {
        FData& d = *m_data;
        *(d.m_resource) = val;
        return *this;
    }
};

FData createData(int value) {
    FData d;
    d.m_resource = new int(value);
    return d;
}

FData createArray(const std::vector<int>& values) {
    FData d;
    d.m_elements.resize(values.size());
    size_t index = 0;
    for (const auto& v : values) {
        d.m_elements[index].m_resource = new int(v);
        index++;
    }
    return d;
}

FRef reference(FData& d) {
    FRef r;
    r.m_data = &d;
    return r;
}

void compute() {
    FData dataStore[4] = {
        createData(1),
        createData(2),
        createData(4),
        createData(4),
    };

    for (auto& data : dataStore) {
        FRef ref = reference(data);
        ref = 100;
    }
}

void compute_with_doublefree() {
    FData dataStore[4] = {
        createData(1),
        createData(2),
        createArray({1, 2, 3}),
        createData(4),
    };

    for (auto& data : dataStore) {
        FData d = data;
    }
}

void compute_with_segfault() {
    FData dataStore[4] = {
        createData(1),
        createData(2),
        createArray({1, 2, 3}),
        createData(4),
    };

    for (auto& data : dataStore) {
        FRef ref = reference(data);
        ref = 100;
    }
}

int main() {
    compute();

#ifdef DOUBLEFREE
    compute_with_doublefree();
#endif

#ifdef SEGFAULT
    compute_with_segfault();
#endif

    return 0;
}