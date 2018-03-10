#include "app_pt.h"
#include "vm_globals.h"

app_pt::app_pt(pid_t parent){
	pt = new page_table_t();
	if(!parent){
		for(unsigned int i = 0; i < VM_ARENA_SIZE/VM_PAGESIZE; ++i){
			ptes[i] = nullptr;
		}
	}else{
		for(unsigned int i = 0; i < VM_ARENA_SIZE/VM_PAGESIZE; ++i){
			ptes[i] = global_data.app_map[parent]->ptes[i];
			++(ptes[i]->num_refs);
		}
		swap_blocks = global_data.app_map[parent]->swap_blocks;
	}
}

app_pt::~app_pt(){
	delete pt;
}