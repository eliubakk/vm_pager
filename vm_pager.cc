#include "vm_pager.h"
#include "vm_globals.h"
#include <cassert>
#include <iostream>

using namespace std;

/*
 * vm_init
 *
 * Called when the pager starts.  It should set up any internal data structures
 * needed by the pager.
 *
 * vm_init is passed the number of physical memory pages and the number
 * of blocks in the swap file.
 */
void vm_init(size_t memory_pages, size_t swap_blocks){
	global_data.memory_pages = memory_pages;
	global_data.max_swap_blocks = swap_blocks;

	//init zero page
	for(unsigned int i = 0; i < VM_PAGESIZE; ++i){
		((char *)vm_physmem)[i] = 0;
	}
}

/*
 * vm_create
 * Called when a parent process (parent_pid) creates a new process (child_pid).
 * vm_create should cause the child's arena to have the same mappings and data
 * as the parent's arena.  If the parent process is not being managed by the
 * pager, vm_create should consider the arena to be empty.
 * Note that the new process is not run until it is switched to via vm_switch.
 * Returns 0 on success, -1 on failure.
 */
int vm_create(pid_t parent_pid, pid_t child_pid){
	if(global_data.app_map.find(parent_pid) == global_data.app_map.end()){
		//parent process not managed by the pager
		parent_pid = 0;
	}else if(!global_data.reserve_blocks(parent_pid)){
		//not enough swap blocks to reserve
		return -1;
	}
	//create new child process
	global_data.app_map[child_pid] = new app_pt(parent_pid);
	return 0;
}

/*
 * vm_switch
 *
 * Called when the kernel is switching to a new process, with process
 * identifier "pid".
 */
void vm_switch(pid_t pid){
	app_pt *app = global_data.app_map[pid];
	page_table_base_register = app->pt;
	//update permission bits (for shared pages)
	for(unsigned int i = 0; i < VM_ARENA_SIZE/VM_PAGESIZE && app->ptes[i] != nullptr; ++i){
		page_table_base_register->ptes[i] = app->ptes[i]->pte;
	}
	global_data.curr_pid = pid;
}

/*
 * vm_fault
 *
 * Called when current process has a fault at virtual address addr.  write_flag
 * is true if the access that caused the fault is a write.
 * Returns 0 on success, -1 on failure.
 */
int vm_fault(const void *addr, bool write_flag){
	unsigned int vpage = (unsigned int)((char*)addr - (char*)VM_ARENA_BASEADDR)/VM_PAGESIZE;
	app_pt* app = global_data.app_map[global_data.curr_pid];
	cout << "vpage: " << vpage << endl;
	//check if valid address
	if(vpage < 0 || vpage >= app->pte_next_index){
		return -1;
	}
	assert(app->ptes[vpage] != nullptr);

	if(!app->ptes[vpage]->resident || (app->pt->ptes[vpage].ppage == 0 && write_flag))
		global_data.load_page(vpage);

	app->ptes[vpage]->reference = 1;
	app->ptes[vpage]->dirty = write_flag? 1 : 0;
	app->ptes[vpage]->pte.read_enable = 1;
	app->ptes[vpage]->pte.write_enable = write_flag? 1 : 0;
	app->pt->ptes[vpage] = app->ptes[vpage]->pte;
	return 0;
}

/*
 * vm_destroy
 *
 * Called when current process exits.  This gives the pager a chance to
 * clean up any resources used by the process.
 */
void vm_destroy(){
	app_pt* app = global_data.app_map[global_data.curr_pid];
	global_data.app_map.erase(global_data.curr_pid);

	//free swap_blocks
	for(unsigned int i = 0; i < global_data.max_swap_blocks; ++i){
		if(app->swap_blocks[i] > 0)
			global_data.swap_blocks[i] = 0;
	}
	global_data.swap_blocks_used -= app->swap_blocks_used;

	//delete app_ptes if last reference
	for(unsigned int i = 0; i < VM_ARENA_SIZE/VM_PAGESIZE; ++i){
		if(app->ptes[i] == nullptr)
			continue;

		if(app->ptes[i]->num_refs == 1 && !app->ptes[i]->resident)
			delete app->ptes[i];
		else
			--(app->ptes[i]->num_refs);
	}
}

/*
 * vm_map
 *
 * A request by the current process for the lowest invalid virtual page in
 * the process's arena to be declared valid.  On success, vm_map returns
 * the lowest address of the new virtual page.  vm_map returns nullptr if
 * the arena is full.
 *
 * If filename is nullptr, block is ignored, and the new virtual page is
 * backed by the swap file, is initialized to all zeroes (from the
 * application's perspective), and private (i.e., not shared with any other
 * virtual page).  In this case, vm_map returns nullptr if the swap file is
 * out of space.
 *
 * If filename is not nullptr, the new virtual page is backed by the specified
 * file at the specified block and is shared with other virtual pages that are
 * mapped to that file and block.  filename is a null-terminated C string and
 * must reside completely in the valid portion of the arena.  In this case,
 * vm_map returns nullptr if filename is not completely in the valid part of
 * the arena.
 * filename is specified relative to the pager's current working directory.
 */
void *vm_map(const char *filename, size_t block){
	if(!filename){
		//swap backed
		if(!global_data.reserve_blocks(0))
			return nullptr;
		app_pt* app = global_data.app_map[global_data.curr_pid];
		app->reserve_blocks(1);
		return app->map_swap_backed();
	}
	return nullptr;
}