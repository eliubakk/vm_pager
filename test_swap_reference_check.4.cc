#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cassert>
#include "vm_app.h"

using namespace std;

int main()
{
    char* swap_backed = (char*)vm_map(nullptr, 0);
    //swap_backed write fault
    strcpy(swap_backed, "data1.bin");
    char* filename = (char*)vm_map(nullptr, 0);

    //write fault 1 -> 3
    strcpy(filename, "data1.bin");
    strcpy(filename + 10, "data2.bin");

    //physmem = [zero, swap_backed = rw, filename (filebacked[1]) = rw]
    //clock = [1 = filename, 1 = swap]
    char* filebacked[5] = {(char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0)};   

    //read fault 1 -> 2
    for (unsigned int i = 0; i < 8; ++i){
        cout << filebacked[0][i];
    }
    cout << endl;
    //physmem = [zero, swap_backed = rw, filename = rw, filebacked[0] = r]
    //clock = [1 = filebacked[0], 1 = filename, 1 = swap_backed]

    filebacked[0][0] = 'a';
    //physmem = [zero, swap_backed = rw, filename = rw, filebacked[0] = rw]
    //clock = [1 = filebacked[0], 1 = filename, 1 = swap_backed]
    cout << filebacked[2][0] << endl;
    //clock sweep
    //vmfault write
    //physmem = [zero, filebacked[2] = r, filename , filebacked[0]]
    //clock = [1 = filebacked[2] ,0 = filename, 0 = filebacked[1]]

    if (fork()){
        filebacked[3][0] = 'a';
        //clock sweep
        //vm fault write
        //physmem = [zero, filebacked[2] = r, filebacked[3] = rw, filebacked[0]]
        //clock = [1 = filebacked[3][0], 1 = filebacked[2] ,0 = filebacked[0]] 
        return 0;
    }
    else{
        filebacked[0][0] = 'b';
        //vm fault write
        //physmem = [zero, filebacked[2] = r, filebacked[3] = rw, filebacked[0] = rw]
        //clock = [1 = filebacked[3][0], 1 = filebacked[2] ,1 = filebacked[0]] 
    }
}