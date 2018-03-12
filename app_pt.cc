#include "app_pt.h"
#include "vm_globals.h"
#include "vm_arena.h"

//REQUIRES: parent != 0, when the pager manages the parent. 
//EFFECTS:  creates a new page_table_t, and
//			if parent != 0, sets ptes equal to parent ptes.	
app_pt::app_pt(pid_t parent){
	pt = new page_table_t();
	swap_blocks_used = 0;
	pte_next_index = 0;
	if(!parent){
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
			if(global_data.app_map[parent]->ptes[i]->file == nullptr){
				//set permission bit to fault for COW.
				global_data.app_map[parent]->pt->ptes[i].write_enable = 0;
				global_data.app_map[parent]->ptes[i]->pte.write_enable = 0;
			}

			//set page data for new app.
			pt->ptes[i] = ptes[i]->pte;

			++(ptes[i]->num_refs);
		}
		pte_next_index = global_data.app_map[parent]->pte_next_index;
		reserve_blocks(global_data.app_map[parent]->swap_blocks_used);
	}
}

//REQUIRES: global_data.reserve_blocks() be called first.
//MODIFIES: global_data.swap_blocks, this.swap_blocks.
//EFFECTS:  sets reserve number of blocks to 1 (reserved).
//RETURNS:  true if there were enough blocks to reserve.
bool app_pt::reserve_blocks(size_t reserve){
	if (reserve > global_data.free_swap_blocks.size()) {
		return false;
	}
	for (size_t i = 0; i < reserve; ++i) {
		reserved_swap_blocks.push(global_data.free_swap_blocks.front());
		global_data.free_swap_blocks.pop();
	}
	swap_blocks_used += reserve;
	return true;
}

//REQUIRES: a new swap_block has been reserved.
//MODIFIES: pt, ptes, swap_blocks
//EFFECTS: creates a new app_pte, and sets swap_block to used.
//RETURNS: virtual address of new swap backed bage.
void* app_pt::map_swap_backed(){
	used_swap_blocks.push(reserved_swap_blocks.front());
	reserved_swap_blocks.pop();
	ptes[pte_next_index] = new app_pte(nullptr, used_swap_blocks.back());
	pt->ptes[pte_next_index] = ptes[pte_next_index]->pte;
	++pte_next_index;
	return (void*)((char*)VM_ARENA_BASEADDR + (pte_next_index - 1)*VM_PAGESIZE);
}

app_pt::~app_pt(){
	delete pt;
}

app_pt::app_pte::app_pte(char* file_in, unsigned int block_in){
	file = file_in;
	block = block_in;
	pte.ppage = 0;
	pte.read_enable = (file_in == nullptr);
	pte.write_enable = 0;
	num_refs = 1;
	dirty = 0b0;
	resident = (file_in == nullptr);
	reference = 0b0;
}