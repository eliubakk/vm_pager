#include "vm_app.h"
#include <cassert>
#include <unistd.h>

using namespace std;

int main(){
	char* swap_backed = (char*)vm_map(nullptr, 0);
	swap_backed[0] = 'a';
	if (fork()){
		vm_yield();
	}
    char* new_page = (char*)vm_map(nullptr, 0);
    for(unsigned int i = 0; i < 4096; ++i){
		new_page[i] = 'b';
	}
	assert(swap_backed[0] == 'a');
    for (unsigned int i = 0; i < 4096; ++i){
		assert(new_page[i] == 'b');
	}
}