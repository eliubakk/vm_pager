#include "vm_app.h"
#include <cassert>

using namespace std;

int main(){
    char* zeros;
	for(unsigned int i = 0; i < 1025; ++i){
		zeros = (char*)vm_map(nullptr, 0);
        zeros[1] = 'a';
	}
    assert(!zeros);
}