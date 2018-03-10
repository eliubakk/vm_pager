#include "vm_globals.h"

vm_globals global_data;

bool vm_globals::reserve_blocks(pid_t parent){
	if(swap_blocks_used + app_map[parent]->swap_blocks > swap_blocks)
		return false;
	swap_blocks_used += app_map[parent]->swap_blocks;
	return true;
}