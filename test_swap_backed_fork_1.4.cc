#include "vm_app.h"
#include <cassert>
#include <iostream>
#include <unistd.h>

using namespace std;

int main(){
	char* swap_backed[5] = {(char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0), (char*)vm_map(nullptr, 0)};	
	//physmem = [zero = r]
	//clock = []

	//write_fault 0 -> 1
	for(unsigned int i = 0; i < 3; ++i){
		swap_backed[i][0] = 'a';
	}
	//physmem = [zero = r, swap_backed[0] = rw, swap_backed[1] = rw, swap_backed[2] = rw]
	//clock = [1 = swap_backed[2], 1 = swap_backed[1], 1 = swap_backed[0]]

	//fork 1 -> 6
	if(fork()){
		//physmem = [zero = r, swap_backed[0] = r, swap_backed[1] = r, swap_backed[2] = r]
		//clock = [1 = swap_backed[2], 1 = swap_backed[1], 1 = swap_backed[0]]

		//write_fault 6 -> 1
		swap_backed[0][0] = 'b';
		//physmem = [zero = r, parent_swap_backed[0] = rw, swap_backed[1], swap_backed[2]]
		//clock = [1 = parent_swap_backed[0], 0 = swap_backed[2], 0 = swap_backed[1]]

		//read_fault 7 -> 6
		cout << swap_backed[1][0] << endl;
		//physmem = [zero = r, parent_swap_backed[0] = rw, swap_backed[1] = r, swap_backed[2]]
		//clock = [1 = parent_swap_backed[0], 0 = swap_backed[2], 1 = swap_backed[1]]

		//evict 7 -> 8
		swap_backed[3][0] = 'a';
		//physmem = [zero = r, parent_swap_backed[0] = rw, swap_backed[1], parent_swap_backed[3] = rw]
		//clock = [1 = parent_swap_backed[3], 0 = swap_backed[1], 1 = parent_swap_backed[0]]

		vm_yield();
		//physmem = [zero = r, parent_swap_backed[0], swap_backed[2] = r, parent_swap_backed[3] = rw]
		//clock = [1 = swap_backed[2], 0 = parent_swap_backed[0], 1 = parent_swap_backed[3]]

		//read_fault 8 -> 9
		cout << swap_backed[1][0] << endl;
		//physmem = [zero = r, swap_backed[1] = r, swap_backed[2] = r, parent_swap_backed[3]]
		//clock = [1 = swap_backed[1], 0 = parent_swap_backed[3], 1 = swap_backed[2]]

		//write_fault 0 -> 1
		swap_backed[4][0] = 'a';
		//physmem = [zero = r, swap_backed[1] = r, swap_backed[2], parent_swap_backed[4] = rw]
		//clock = [1 = parent_swap_backed[4], 0 = swap_backed[2], 1 = swap_backed[1]]

		vm_yield();
		//physmem = [zero = r, parent_swap_backed[1], child_swap_backed[3], child_swap_backed[1]]
		//clock = [1 = child_swap_backed[1], 0 = child_swap_backed[3], 0 = parent_swap_backed[1]]

		//read_fault 5 -> 4
		cout << swap_backed[1][0] << endl;
		//physmem = [zero = r, parent_swap_backed[1] = r, child_swap_backed[3], child_swap_backed[1]]
		//clock = [1 = child_swap_backed[1], 0 = child_swap_backed[3], 1 = parent_swap_backed[1]]

		//read_fault 8 -> 9
		//write fault 9 -> 1
		//write fault 9 -> 4
		swap_backed[2][0] = 'c';
		//physmem = [zero = r, parent_swap_backed[2] = rw, child_swap_backed[2], child_swap_backed[1]]
		//clock = [1 = parent_swap_backed[2], 0 = child_swap_backed[1], 1 = child_swap_backed[2]]
	}else{
		//physmem = [zero = r, parent_swap_backed[0], swap_backed[1], parent_swap_backed[3]]
		//clock = [1 = parent_swap_backed[3], 0 = swap_backed[1], 1 = parent_swap_backed[0]]

		//read_fault 8->9
		cout << swap_backed[2][0] << endl;
		//physmem = [zero = r, parent_swap_backed[0], swap_backed[2] = r, parent_swap_backed[3]]
		//clock = [1 = swap_backed[2], 0 = parent_swap_backed[0], 1 = parent_swap_backed[3]]

		vm_yield();
		//physmem = [zero = r, swap_backed[1] = r, swap_backed[2], parent_swap_backed[4]]
		//clock = [1 = parent_swap_backed[4], 0 = swap_backed[2], 1 = swap_backed[1]]

		//evict 10 -> 8
		swap_backed[3][0] = 'a';
		//physmem = [zero = r, swap_backed[1], child_swap_backed[3] = rw, parent_swap_backed[4]]
		//clock = [1 = child_swap_backed[3], 0 = swap_backed[1], 1 = parent_swap_backed[4]]

		//read_fault 10 -> 9
		cout << swap_backed[1][0] << endl;
		//physmem = [zero = r, swap_backed[1] = r, child_swap_backed[3] = rw, parent_swap_backed[4]]
		//clock = [1 = child_swap_backed[3], 1 = swap_backed[1], 1 = parent_swap_backed[4]]

		//write_fault 9 -> 4
		//write_fault 9 -> 1
		swap_backed[1][0] = 'c';
		//physmem = [zero = r, parent_swap_backed[1], child_swap_backed[3], child_swap_backed[1] = rw]
		//clock = [1 = child_swap_backed[1], 0 = child_swap_backed[3], 0 = parent_swap_backed[1]]

		vm_yield();
		//physmem = [zero = r, empty, child_swap_backed[2] = r, child_swap_backed[1]]
		//clock = [0 = child_swap_backed[1], 1 = child_swap_backed[2]]

		swap_backed[1][0] = 'b';
		//physmem = [zero = r, empty, child_swap_backed[2] = r, child_swap_backed[1] = rw]
		//clock = [1 = child_swap_backed[1], 1 = child_swap_backed[2]]

		swap_backed[0][0] = 'c';
		//physmem = [zero = r, child_swap_backed[0] = rw, child_swap_backed[2] = r, child_swap_backed[1] = rw]
		//clock = [1 = child_swap_backed[0], 1 = child_swap_backed[1], 1 = child_swap_backed[2]]
	}
}