#include "vm_app.h"
#include <cassert>
#include <iostream>

using namespace std;

int main(){
	char* swap_backed = (char*)vm_map(nullptr, 0);	

	for(unsigned int i = 0; i < 4096; ++i){
		swap_backed[i] = 'a';
	}
	for(unsigned int i = 0; i < 4096; ++i){
		//cout << "swap_backed[" << i << "] = " << swap_backed[i] << endl;
		assert(swap_backed[i] == 'a');
	}
}