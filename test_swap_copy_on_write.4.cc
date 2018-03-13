#include "vm_app.h"
#include <cassert>
#include <unistd.h>

using namespace std;

int main(){
	char* swap_backed = (char*)vm_map(nullptr, 0);
	swap_backed[0] = 'a';
	if (fork()){
		vm_yield();
		assert(swap_backed[0] == 'a');
		swap_backed[0] = 'b';
		return 0;
	}
	for(unsigned int i = 0; i < 4096; ++i){
		swap_backed[i] = 'a';
	}
	for (unsigned int i = 0; i < 4096; ++i){
		assert(swap_backed[i] == 'a');
	}
}