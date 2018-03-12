#include "vm_globals.h"
#include <iostream>

using namespace std;

vm_globals global_data;

//Modifies: global_data.swap_blocks_used
//Effects: if parent == 0, reserves one swap block,
//		   otherwise reserves number of swap_blocks parent has.
//Returns: true if enough swap blocks, else false.
bool vm_globals::reserve_blocks(pid_t parent){
	int reserve = 1;
	if(parent)
		reserve = app_map[parent]->swap_blocks_used;

	if(swap_blocks_used + reserve > max_swap_blocks)
		return false;

	swap_blocks_used += reserve;
	return true;
}

//REQUIRES: page be a vaild pointer and not be in physmem.
//MODIFIES: physmem, page, global_data.clock
//EFFECTS: loads page into physmem, evicts a page to disk if mem is full
void vm_globals::load_page(unsigned int vpage){
	app_pt* app = app_map[curr_pid];
	app_pt::app_pte *page = app->ptes[vpage];
	//unsigned int pindex = 0;
	unsigned int ppage = 1;
	app_pt::app_pte *evicted = nullptr;
	//physmem is not full
	if(clock.size() < (memory_pages - 1)){
		cout << "free space in mem" << endl;
		ppage += clock.size();
	}
	else{
		while(evicted == nullptr){
			evicted = clock.front();
			clock.pop_front();
			if(evicted->reference){
				evicted->reference = 0;
				evicted->pte.read_enable = 0;
				evicted->pte.write_enable = 0;
				for(unsigned int i = 0; app->ptes[i] != nullptr && i < VM_ARENA_SIZE/VM_PAGESIZE; ++i){
					if(app->ptes[i] == evicted){
						app->ptes[i]->pte = evicted->pte;
						break;
					}
				}
				clock.push_back(evicted);
				evicted = nullptr;
			}else{
				if(evicted->dirty && !(evicted->num_refs == 0 && evicted->file == nullptr)){
					file_write(evicted->file, evicted->block, (void*)((char*)vm_physmem + (evicted->pte.ppage * VM_PAGESIZE)));
					evicted->dirty = 0;
				}
				evicted->pte.read_enable = 0;
				evicted->pte.write_enable = 0;
				for(unsigned int i = 0; app->ptes[i] != nullptr && i < VM_ARENA_SIZE/VM_PAGESIZE; ++i){
					if(app->ptes[i] == evicted){
						app->ptes[i]->pte = evicted->pte;
						break;
					}
				}
				evicted->resident = 0;
				ppage = evicted->pte.ppage;
				if(evicted->num_refs == 0){
					delete evicted;
				}
			}
		}
	}
	cout << "ppage: " << ppage << endl;
	page->pte.ppage = ppage;
	file_read(page->file, page->block, (void*)((char*)vm_physmem + (ppage * VM_PAGESIZE)));
	page->reference = 0;
	page->resident = 1;
	clock.push_back(page);
}