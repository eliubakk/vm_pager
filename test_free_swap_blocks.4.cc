#include "vm_app.h"
#include <cassert>
#include <unistd.h>

using namespace std;

int main(){
    char* zeros;
	for(unsigned int i = 0; i < 511; ++i){
		zeros = (char*)vm_map(nullptr, 0);
	}
    if(fork()) {
        vm_yield();
    }
    else {
        while(zeros) {
            zeros = (char *)vm_map(nullptr, 0);
        }
        vm_yield();
        zeros = (char *)vm_map(nullptr, 0);
        assert(!zeros);
    }
}