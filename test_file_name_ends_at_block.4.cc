#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
#include <cassert>

using namespace std;

int main()
{
    /* Allocate swap-backed page from the arena */
    char *filename1 = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */

    filename1[4081] = 's';
    filename1[4082] = 'h';
    filename1[4083] = 'a';
    filename1[4084] = 'k';
    filename1[4085] = 'e';
    filename1[4086] = 's';
    filename1[4087] = 'p';
    filename1[4088] = 'e';
    filename1[4089] = 'a';
    filename1[4090] = 'r';
    filename1[4091] = 'e';
    filename1[4092] = '.';
    filename1[4093] = 't';
    filename1[4094] = 'x';
    filename1[4095] = 't';

    /* Map a page from the specified file */
    char *p = (char *) vm_map ((filename1 + 4081), 0);

    assert(p == nullptr);
}