#include "vm_app.h"
#include <cassert>
#include <iostream>

using namespace std;

int main(){
	char* swap_backed[5] = {(char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0)};	

	for(unsigned int i = 0; i < 4; ++i){
		swap_backed[i][0] = 'a';
	}
	assert(swap_backed[1][0] == 'a');
	assert(swap_backed[0][0] == 'a');
	swap_backed[4][0] = 'a';
	swap_backed[1][1] = 'b';
	assert(swap_backed[0][0] == 'a');
	assert(swap_backed[2][0] == 'a');
	swap_backed[0][1] = 'b';
	swap_backed[3][1] = 'b';
	assert(swap_backed[1][1] == 'b');
	swap_backed[1][2] = 'c';
	swap_backed[4][1] = 'b';
	swap_backed[3][2] = 'c';
}