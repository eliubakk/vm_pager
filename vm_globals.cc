#include "vm_globals.h"
#include <iostream>
#include <bitset>

using namespace std;

vm_globals global_data;

vm_globals::vm_globals(){
	zero_page = new app_pt::app_pte("", 0);
	zero_page->num_refs = 0;
	zero_page->resident = 1;
	curr_pid = 0;
	memory_pages = 0;
	max_swap_blocks = 0;
	swap_blocks_used = 0;
}

vm_globals::~vm_globals(){
	delete zero_page;
}

//Modifies: global_data.swap_blocks_used
//Effects: if parent == 0, reserves one swap block,
//		   otherwise reserves number of swap_blocks parent has.
//Returns: true if enough swap blocks, else false.
bool vm_globals::reserve_blocks(pid_t parent){
	size_t reserve = 1;
	if(parent)
		reserve = app_map[parent]->swap_blocks_used;

	if(swap_blocks_used + reserve > max_swap_blocks)
		return false;

	swap_blocks_used += reserve;
	return true;
}

//REQUIRES: vpage be mapped, and not be in physmem.
//MODIFIES: physmem, vpage, global_data.clock
//EFFECTS: loads page into physmem, evicts a page to disk if mem is full
//RETURNS: false on file_read or file_write fail, otherwise true.
bool vm_globals::load_page(unsigned int vpage, char* buffer){
	app_pt* app = app_map[curr_pid];
	app_pt::app_pte *page = app->ptes[vpage];
	unsigned int ppage = 1;
	app_pt::app_pte *evicted = nullptr;
	//physmem is not full
	if(clock.size() < (memory_pages - 1)){
		vector<bool> resident(memory_pages, false);
		resident[0] = true;
		for(auto it: clock){
			resident[it->pte.ppage] = true;
		}

		//find first ppage that is not resident and break
		for(unsigned int i = 0; i < memory_pages; ++i){
			if(!resident[i]){
				ppage = i;
				break;
			}
		}
	}
	// physmem full - clock eviction needed
	else{
		//set evicted to LRU
		while(evicted == nullptr){
			evicted = clock.front();
			clock.pop_front();

			//reset read and write enable to fault on next reference
			evicted->pte.read_enable = 0;
			evicted->pte.write_enable = 0;

			//if evicted is a page in the running process, update external PTE
			for(unsigned int i = 0; app->ptes[i] != nullptr && i < VM_ARENA_SIZE/VM_PAGESIZE; ++i){
				if(app->ptes[i] == evicted){
					app->pt->ptes[i] = evicted->pte;
					break;
				}
			}

			//if recently referenced, give second chance
			if(evicted->reference){
				evicted->reference = 0;
				clock.push_back(evicted);
				evicted = nullptr;
			}
		}

		//if dirty and not non-referenced swap-backed page, write to disk
		if(evicted->dirty && !(evicted->num_refs == 0 && evicted->file == "")){
			const char *filename = (evicted->file == "")? nullptr : evicted->file.c_str();
			if(file_write(filename, evicted->block, (void*)((char*)vm_physmem + (evicted->pte.ppage * VM_PAGESIZE))) == -1)
				return false;
			evicted->dirty = 0;
		}
		evicted->resident = 0;
		ppage = evicted->pte.ppage;

		//if no more references to this page, delete page
		if(evicted->num_refs == 0){
			if(evicted->file != "")
				global_data.file_blocks[evicted->file].erase(evicted->block);
			delete evicted;
		}
	}

	//read requested page into memory
	if ((page->pte.ppage == 0 && page->file == "") || buffer != nullptr){
		for (unsigned int i = 0; i < VM_PAGESIZE; ++i){
			//copy on write, or init to zero
			((char*)vm_physmem + ppage * VM_PAGESIZE)[i] = buffer? buffer[i] : 0;
		}
	}
	else{
		const char *filename = page->file == "" ? nullptr : page->file.c_str();
		if(file_read(filename, page->block, (void*)((char*)vm_physmem + (ppage * VM_PAGESIZE))) == -1){
			page->reference = 0;
			page->resident = 0;
			return false;	
		}
	}
	page->pte.ppage = ppage;
	page->reference = 0;
	page->resident = 1;
	clock.push_back(page);
	return true;
}

//MODIFIES: clock
//EFFECTS: if page is in the clock, it gets removed, order does not change.
void vm_globals::remove_from_clock(app_pt::app_pte* page){
	for(auto it = clock.begin(); it != clock.end(); ++it){
		if(*it == page){
			clock.erase(it);
			break;
		}
	}
}