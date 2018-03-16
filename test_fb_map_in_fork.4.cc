#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cassert>
#include "vm_app.h"

using namespace std;

int main()
{
    char *filename = (char *) vm_map(nullptr, 0);
    strcpy(filename, "shakespeare.txt");
    char *file_backed3 = (char *) vm_map (filename, 0);
    if (fork()){
        char *file_backed = (char *) vm_map (filename, 0);
        file_backed[0] = 'a';
        file_backed[1] = 'b';
        vm_yield();
        if (fork()){
            assert(file_backed3[0] == 'w');
            assert(file_backed3[1] == 'x');
            assert(file_backed3[2] == 'y');
            assert(file_backed3[3] == 'z');
            file_backed[0] = 'i';
            vm_yield();
            assert(file_backed[0] == 'j');
            assert(file_backed[1] == 'h');
            assert(file_backed[2] == 'y');
            assert(file_backed[3] == 'z');
        }
        else{
            assert(file_backed3[0] == 'f');
            assert(file_backed3[1] == 'h');
            file_backed[0] = 'j';
        }
    }
    else{
        char *file_backed2 = (char *) vm_map (filename, 0);
        assert(file_backed2[0] == 'a');
        assert(file_backed2[1] == 'b');
        file_backed2[0] = 'w';
        file_backed2[1] = 'x';
        file_backed2[2] = 'y';
        file_backed3[3] = 'z';
        vm_yield();
        assert(file_backed2[0] == 'i');
        file_backed3[0] = 'f';
        file_backed2[1] = 'h';
        assert(file_backed2[2] == 'y');
    }
}