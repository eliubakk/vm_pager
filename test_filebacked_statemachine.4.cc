#include "vm_app.h"
#include <cassert>
#include <cstring>
#include <iostream>

using namespace std;

int main(){
	char* swap_backed = (char*)vm_map(nullptr, 0);
	//swap_backed write fault
	strcpy(swap_backed, "data1.bin");
	char* filename = (char*)vm_map(swap_backed, 1);

	//write fault 1 -> 3
	strcpy(filename, "data1.bin");
	strcpy(filename + 10, "data2.bin");

	//physmem = [zero, swap_backed = rw, filename (filebacked[1]) = rw]
	//clock = [1 = filename (filebacked[0]), 1 = swap_backed]
	char* filebacked[5] = {(char*)vm_map(filename, 0), (char*)vm_map(filename, 1), (char*)vm_map(filename, 2), (char*)vm_map(filename + 10, 0), (char*)vm_map(filename + 10, 1)};	

	//read fault 1 -> 2
	for (unsigned int i = 0; i < 8; ++i){
		cout << filebacked[0][i];
	}
	cout << endl;
	//physmem = [zero, swap_backed = rw, filename (filebacked[1]) = rw, filebacked[0] = r]
	//clock = [1 = filebacked[0], 1 = filebacked[1], 1 = swap_backed]
	
	//write fault 2 -> 3
	for (unsigned int i = 0; i < 8; ++i){
		filebacked[0][i] = (char)('a' + i);
	}
	//physmem = [zero, swap_backed = rw, filename (filebacked[1]) = rw, filebacked[0] = rw]
	//clock = [1 = filebacked[0], 1 = filebacked[1], 1 = swap_backed]

	//clock_sweep 3 -> 5
	//read fault 1 -> 2
	for (unsigned int i = 0; i < 8; ++i){
		cout << filebacked[2][i];
	}
	cout << endl;
	//physmem = [zero, filebacked[2] = r, filename (filebacked[1]), filebacked[0]]
	//clock = [1 = filebacked[2], 0 = filebacked[0], 0 = filebacked[1]]

	//read fault 5 -> 3
	for (unsigned int i = 0; i < 8; ++i){
		cout << filebacked[0][i];
	}
	cout << endl;
	//physmem = [zero, filebacked[2] = r, filename (filebacked[1]), filebacked[0] = rw]
	//clock = [1 = filebacked[2], 1 = filebacked[0], 0 = filebacked[1]]

	//write fault 5 -> 3
	for (unsigned int i = 0; i < 8; ++i){
		filebacked[1][i] = (char)('a' + i);
	}
	//physmem = [zero, filebacked[2] = r, filename (filebacked[1]) = rw, filebacked[0] = rw]
	//clock = [1 = filebacked[2], 1 = filebacked[0], 1 = filebacked[1]]

	//clock evict 3 -> 1
	//clock sweep 3 -> 5
	//clock sweep 2 -> 4
	//read fault 1 -> 2
	for (unsigned int i = 0; i < 8; ++i){
		cout << filebacked[3][i];
	}
	cout << endl;
	//physmem = [zero, filebacked[2], filebacked[3] = r, filebacked[0]]
	//clock = [1 = filebacked[3], 0 = filebacked[2], 0 = filebacked[0]]

	//clock evict 5 -> 1
	//read fault 1 -> 2
	for (unsigned int i = 0; i < 8; ++i){
		cout << filebacked[4][i];
	}
	cout << endl;
	//physmem = [zero, filebacked[2], filebacked[3] = r, filebacked[4] = r]
	//clock = [1 = filebacked[4], 1 = filebacked[3], 0 = filebacked[2]]

	//read fault 4 -> 2
	for (unsigned int i = 0; i < 8; ++i){
		cout << filebacked[2][i];
	}
	cout << endl;
	//physmem = [zero, filebacked[2] = r, filebacked[3] = r, filebacked[4] = r]
	//clock = [1 = filebacked[4], 1 = filebacked[3], 1 = filebacked[2]]

	//clock evict 2 -> 1
	//clock sweep 2 -> 4
	//write fault 1 -> 3
	for (unsigned int i = 0; i < 8; ++i){
		filename[i] = (char)('a' + i);
	}
	//physmem = [zero, filename (filebacked[1]) = rw, filebacked[3], filebacked[4]]
	//clock = [1 = filename (filebacked[1]), 0 = filebacked[4], 0 = filebacked[3]]

	//write fault 4 -> 3
	for (unsigned int i = 0; i < 8; ++i){
		filebacked[3][i] = (char)('a' + i);
	}
	//physmem = [zero, filename (filebacked[1]) = rw, filebacked[3] = rw, filebacked[4]]
	//clock = [1 = filename (filebacked[1]), 0 = filebacked[4], 1 = filebacked[3]]

	//read fault 1 -> 2
	//clock sweep 3 -> 5
	//clock evict 4 ->1
	for (unsigned int i = 0; i < 8; ++i){
		cout << filebacked[2][i];
	}
	cout << endl;
	//physmem = [zero, filename (filebacked[1]) = rw, filebacked[3], filebacked[2] = r]
	//clock = [1 = filebacked[2], 0 = filebacked[3], 1 = filename (filebacked[1])]
}