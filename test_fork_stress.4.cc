#include "vm_app.h"
#include <cassert>
#include <unistd.h>

using namespace std;

int main(){
    int id = 0;
    static int static_id = 0;
	char* swap_backed = (char*)vm_map(nullptr, 0);
    char* zero = (char*)vm_map(nullptr, 34);
	swap_backed[0] = 'a';
	if(!fork())
        id = ++static_id;
    if (fork()) {
        swap_backed[1] = 'b';
	    vm_yield();
        assert(swap_backed[1] == 'b');
    } else {
        id = ++static_id;
        zero[0] = 'c';
    }
    if(!fork()) {
        id = ++static_id;
        zero[1] = 'd';
    }
    
	assert(swap_backed[0] == 'a');

    switch (id) {
        case 0:
        case 1:
            assert(swap_backed[1] != 'b');
            assert(zero[0] != 'c');
            assert(zero[1] != 'd');
            break;
        case 2:
        case 3:
            assert(zero[0] == 'c');
            assert(swap_backed[1] != 'b');
            assert(zero[1] != 'd');
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            assert(zero[1] == 'd');
            break;
    }

}