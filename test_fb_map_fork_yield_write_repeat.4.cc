#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cassert>
#include "vm_app.h"

using namespace std;

int main()
{
    char *filename = (char *) vm_map(nullptr, 0);
    strcpy(filename, "data1.bin");
    char *a = (char *) vm_map (filename, 0);
    a[150] = 'a';
    char *a2 = (char *) vm_map (nullptr, 0);
    fork();
    a[0] = 'b';
    a2[0] = 'c';
    char *a3 = (char *) vm_map(nullptr, 0);
    vm_yield();
    a[0] = 'd';
    a2[0] = 'e';
    a3[0] = 'f';
    char *a4 = (char *) vm_map(nullptr, 0);
    a[0] = 'g';
    a2[0] = 'h';
    a3[0] = 'i';
    a4[0] = 'j';
    char *a5 = (char *) vm_map(nullptr, 0);
    a[0] = 'k';
    a2[0] = 'l';
    a3[0] = 'm';
    a4[0] = 'n';
    a5[0] = 'o';
    vm_yield();
    char *a6 = (char *) vm_map(filename, 0);
    a6 = a5;
    a6[0] = 'p';
    a5 = a;
    char *a7 = (char *) vm_map(nullptr, 0);
    fork();
    a7[0] = 'q';
    char *a8 = (char *) vm_map(filename, 0);
    vm_yield();
    *a8 = *a4;
    a8[0] = 'r';
    fork();
    assert(a[150] == 'a');
    strcpy(a4, "482 is fun :)");
    fork();
}