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
    char *p = (char *) vm_map (filename, 0);
    if (fork()){
        for (int i = 0; i < 2561; i++) {
            if (i < 1000){
                cout << p[i];
                p[i] = 'a';
            }
            else if (i == 1000)
                vm_yield();
            else
                assert(p[i] == 'a');
        }
    }
    else{
        for (int i = 0; i < 2561; i++) {
            if (i < 1000)
                assert(p[i] == 'a');
            else{
                cout << p[i];
                p[i] = 'a';
            }
        }  
    }
    for (int i = 0; i < 2561; i++){
        assert(p[i] == 'a');
    }
}