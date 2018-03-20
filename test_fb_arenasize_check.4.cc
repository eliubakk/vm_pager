#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cassert>
#include "vm_app.h"

using namespace std;

int main()
{
    char* swapbacked = (char*) vm_map(nullptr, 0);
    strcpy(swapbacked, "data1.bin");
    for (unsigned int i = 0; i < 5000; ++i){
        if ((char*)vm_map(swapbacked, 0) == nullptr)
            assert(false);
    }
}