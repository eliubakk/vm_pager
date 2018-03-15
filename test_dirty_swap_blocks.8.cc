#include "vm_app.h"
#include <cassert>

int main() {
    char * swap_backed[8];
    for (int i = 0; i < 8; ++i) {
        swap_backed[i] = (char *)vm_map(nullptr, 0);
        swap_backed[i][0] = i;
    }

    swap_backed[1][0] = 'a';
    swap_backed[3][0] = 'b';
    swap_backed[5][0] = 'c';

    assert(swap_backed[0][0] == 0);
    swap_backed[2][0] = 'd';
}