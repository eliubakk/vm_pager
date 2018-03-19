#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
    /* Allocate swap-backed page from the arena */
    char *filename1 = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(filename1, "shakespeare.txt");
    for(unsigned int i = 0; i < 3; ++i){
        char *swap = (char *)vm_map(nullptr, 0);
        swap[0] = 'a';
    }

    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename1, 0);

    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
	   cout << p[i];
    }
}