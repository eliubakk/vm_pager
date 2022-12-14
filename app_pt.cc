#include "app_pt.h"
#include "vm_globals.h"
#include "vm_arena.h"
#include <iostream>

using namespace std;

//REQUIRES: parent != 0, when the pager manages the parent. 
//EFFECTS:  creates a new page_table_t, and
//			if parent != 0, sets ptes equal to parent ptes.	
app_pt::app_pt(pid_t parent){
	pt = new page_table_t();
	swap_blocks_used = 0;
	pte_next_index = 0;
	if(!parent){
		//init everything to zero
		for(unsigned int i = 0; i < VM_ARENA_SIZE/VM_PAGESIZE; ++i){
			pt->ptes[i].ppage = 0;
			pt->ptes[i].read_enable = 0;
			pt->ptes[i].write_enable = 0;
			ptes[i] = nullptr;
		}
	}else{
		for(unsigned int i = 0; i < VM_ARENA_SIZE/VM_PAGESIZE; ++i){
			ptes[i] = global_data.app_map[parent]->ptes[i];

			//page not vaild
			if(ptes[i] == nullptr){
				pt->ptes[i].ppage = 0;
				pt->ptes[i].read_enable = 0;
				pt->ptes[i].write_enable = 0;
				continue;
			}

			//swap-backed
			if(global_data.app_map[parent]->ptes[i]->file == ""){
				//set permission bit to fault for COW.
				global_data.app_map[parent]->pt->ptes[i].write_enable = 0;
				global_data.app_map[parent]->ptes[i]->pte.write_enable = 0;
			}

			//set page data for new app.
			pt->ptes[i] = ptes[i]->pte;
			if(ptes[i] != global_data.zero_page)
				++(ptes[i]->num_refs);
		}
		pte_next_index = global_data.app_map[parent]->pte_next_index;
		reserve_blocks(global_data.app_map[parent]->swap_blocks_used);
	}
}

//REQUIRES: global_data.reserve_blocks() be called first.
//MODIFIES: swap_blocks_used.
//EFFECTS:  increases swap_blocks_used by reserve.
//RETURNS:  true if there were enough blocks to reserve.
bool app_pt::reserve_blocks(size_t reserve){
	swap_blocks_used += reserve;
	return true;
}

//MODIFIES: pt, ptes
//EFFECTS: Pushes changes to the internal pte, to the external pte
void app_pt::update_external_pt(){
	for(unsigned int i = 0; i < VM_ARENA_SIZE/VM_PAGESIZE && ptes[i] != nullptr; ++i){
		pt->ptes[i] = ptes[i]->pte;
	}
}

//REQUIRES: arena not be full, and a new swap_block has been reserved.
//MODIFIES: pt, ptes, free_swap_blocks
//EFFECTS: maps to zero page or creates a new app_pte, 
//         and uses a free_swap_block.
//RETURNS: virtual address of new swap backed bage.
void* app_pt::map_swap_backed(int index){
	if(index == -1){
		//vm_map called
		index = pte_next_index++;
		ptes[index] = global_data.zero_page;
	}
	else{
		//write to zero page or copy on write
		ptes[index] = new app_pte("", global_data.free_swap_blocks.front());
		global_data.free_swap_blocks.pop();		
	}
	pt->ptes[index] = ptes[index]->pte;
	return (void*)((char*)VM_ARENA_BASEADDR + (index)*VM_PAGESIZE);
}

//REQUIRES: arena not be full
//MODIFIES: pt, ptes, file_blocks, pte_next_index
//EFFECTS: creates a new app_pte one for filename and block doesn't exist.
//		   otherwise it increases num_refs. Puts the app_pte into ptes.
//RETURNS: virtual address of new filebacked page.
void* app_pt::map_file_backed(string filename, size_t block) {
	unsigned int index = pte_next_index++;
	//check if this file and block has already been mapped
	if (global_data.file_blocks[filename].find(block) == global_data.file_blocks[filename].end()) {
		global_data.file_blocks[filename][block] = new app_pte(filename, block);
	} else {
		//increase num refs to this file and block
		++(global_data.file_blocks[filename][block]->num_refs);
	}

	ptes[index] = global_data.file_blocks[filename][block];
	pt->ptes[index] = ptes[index]->pte;
	return (void*)((char*)VM_ARENA_BASEADDR + (index)*VM_PAGESIZE);
}


app_pt::~app_pt(){
	delete pt;
}

app_pt::app_pte::app_pte(string file_in, unsigned int block_in) : file(file_in) {
	block = block_in;
	pte.ppage = 0;
	pte.read_enable = (file_in == "");
	pte.write_enable = 0;
	num_refs = 1;
	dirty = 0b0;
	resident = 0;
	reference = 0b0;
}

