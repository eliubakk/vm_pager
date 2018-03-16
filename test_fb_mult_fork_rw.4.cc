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
            if (i < 1000){ // set first 1000 to 'a'
                cout << p[i];
                p[i] = 'a';
            }
            else if (i == 1000){
                vm_yield();
                if (fork()){
                    for (int i = 0; i < 2561; ++i){
                        assert(p[i] == 'a');
                    }
                    vm_yield();
                }
                else{
                    if (fork()){
                        for (int i = 0; i < 2561; ++i){
                            p[i] = 'b';
                        }
                        for (int i = 0; i < 2561; ++i){
                            assert(p[i] == 'b');
                        }
                    }
                    else{
                        return 0;
                    }
                    vm_yield();
                    for (int i = 0; i < 2561; i++){
                        if (i <= 1000){
                            assert(p[i] == 'b');
                        }
                        else{
                            assert(p[i] == 'a');
                        }
                    }
                    return 0;
                }
            }
            else{
                assert(p[i] == 'b');
                p[i] = 'a';
            }
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
        return 0;  
    }
    for (int i = 0; i < 2561; i++){
        if (i <= 1000){
            assert(p[i] == 'b');
        }
        else{
            assert(p[i] == 'a');
        }
    }
}