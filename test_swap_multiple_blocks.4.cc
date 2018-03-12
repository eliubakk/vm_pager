#include "vm_app.h"
#include <cassert>

using namespace std;

int main(){
	char* swap_backed = (char*)vm_map(nullptr, 0);
	char* swap_backed2 = (char*)vm_map(nullptr, 0);
	char* swap_backed3 = (char*)vm_map(nullptr, 0);
	char* swap_backed4 = (char*)vm_map(nullptr, 0);

	for(unsigned int i = 0; i < 4096; ++i){
		swap_backed[i] = 'a';
		swap_backed2[i] = 'b';
		swap_backed3[i] = 'c';
		swap_backed4[i] = 'd';		
	}
	for (unsigned int i = 0; i < 4096; ++i){
		assert(swap_backed[i] == 'a');
		assert(swap_backed2[i] == 'b');
		assert(swap_backed3[i] == 'c');
		assert(swap_backed4[i] == 'd');
	}
}