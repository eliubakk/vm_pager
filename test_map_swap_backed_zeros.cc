#include "vm_app.h"
#include <cassert>

using namespace std;

int main(){
	char* zeros = (char*)vm_map(nullptr, 0);
	char* zeros_two = (char*)vm_map(nullptr, 0);

	for(unsigned int i = 0; i < 4096; ++i){
		assert(!zeros[i]);
		assert(!zeros_two[i]);
	}
}