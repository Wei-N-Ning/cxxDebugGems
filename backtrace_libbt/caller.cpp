
#include <vector>

using namespace std;

extern void sut();

void searchAndCall(const vector<int>& elems, int index) {
    if (index < 0) {
        return;
    }
    if (elems[index] == 0) {
        sut();
    }
    searchAndCall(elems, --index);
}

void callsite() {
    searchAndCall({0xDE, 0xAD, 0xBE, 0, 0xEF}, 3);
}

int main() {
    callsite();
    return 0;
}
