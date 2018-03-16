#include "vm_app.h"
#include <cassert>
#include <cstring>
#include <unistd.h>

using namespace std;

int main(){
	char* swap_backed = (char*)vm_map(nullptr, 0);
	strcpy(swap_backed, "data1.bin");
	strcpy(swap_backed + 11, "data2.bin");
	char* file_backed1 = (char*)vm_map(swap_backed, 1);
	char* file_backed4 = (char*)vm_map(swap_backed, 4);
	char* file_backed3 = (char*)vm_map(swap_backed + 11, 0);
	if (fork()){
		char local = file_backed3[0];
		assert(local == file_backed3[0]);
		file_backed1[5] = 'p';
		vm_yield();
		assert(file_backed4[8] == 'q');
		return 0;
	}
	assert(file_backed1[5] == 'p');
	file_backed4[8] = 'q';
	for(unsigned int i = 0; i < 10; ++i){
		swap_backed[i] = 'a';
	}
}