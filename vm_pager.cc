#include "vm_pager.h"
#include "vm_globals.h"
#include <cassert>
#include <iostream>
#include <sstream>

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

	//initialize free_swap_blocks
	for (unsigned int i = 0; i < swap_blocks; ++i) {
		global_data.free_swap_blocks.push(i);
	}

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

	//update permission bits that may have changed
	app->update_external_pt();

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

	//check if valid address
	if(addr < VM_ARENA_BASEADDR || addr >= ((char*)VM_ARENA_BASEADDR + VM_ARENA_SIZE) || vpage >= app->pte_next_index)
		return -1;
	assert(app->ptes[vpage] != nullptr);
	
	//if writing to zero page, allocate page
	if(app->ptes[vpage] == global_data.zero_page && write_flag){
		app->map_swap_backed(vpage);
	}

	//load page from disk
	if(!app->ptes[vpage]->resident){
		if(!global_data.load_page(vpage))
			return -1;	
	}

	//in memory and has been referenced.
	app->ptes[vpage]->reference = 1;
	app->ptes[vpage]->pte.read_enable = 1;

	//Copy on write - swapbacked only
	if(app->ptes[vpage]->file == "" && write_flag && app->ptes[vpage]->num_refs > 1){
		char buffer[VM_PAGESIZE];
		for(unsigned int i = 0; i < VM_PAGESIZE; ++i){
			//Read page from memory into kernel adderess space.
			buffer[i] = ((char*)vm_physmem + app->ptes[vpage]->pte.ppage * VM_PAGESIZE)[i];
		}

		//Page can be written to if dirty and no other pages reference it
		if(--(app->ptes[vpage]->num_refs) == 1){
			app->ptes[vpage]->pte.write_enable = (app->ptes[vpage]->resident && app->ptes[vpage]->dirty);
		}

		//Getting new virtual page to write buffer to.
		app->map_swap_backed(vpage);

		//Load page into memory and copy buffer to it.
		if(!global_data.load_page(vpage, buffer))
			return -1;

		//page has been referenced and can be read from.
		app->ptes[vpage]->reference = 1;
		app->ptes[vpage]->pte.read_enable = 1;
	}

	//if just written to, dirty
	app->ptes[vpage]->dirty |=  write_flag;

	//enable write when page is not shared swapbacked, and is dirty 
	app->ptes[vpage]->pte.write_enable = 
		(app->ptes[vpage]->file == "" && app->ptes[vpage]->num_refs > 1)? 0 : app->ptes[vpage]->dirty;
	
	//updated external page table to reflect internal changes 
	app->update_external_pt();
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
	global_data.swap_blocks_used -= app->swap_blocks_used;

	//delete app_ptes if last reference
	for(unsigned int i = 0; i < VM_ARENA_SIZE/VM_PAGESIZE; ++i){
		//don't delete zero page.
		if(app->ptes[i] == nullptr || app->ptes[i] == global_data.zero_page){
			continue;
		}

		//If only process referencing vpage and is not in memory, delete vpage.
		if(--(app->ptes[i]->num_refs) == 0 && !app->ptes[i]->resident) {
			if (app->ptes[i]->file != "") {
				//file backed, not in memory, delete vpage
				global_data.file_blocks[app->ptes[i]->file].erase(app->ptes[i]->block);
				delete app->ptes[i];
			} else {
				//swap backed not in memory, delete vpage, free swapblock
				global_data.free_swap_blocks.push(app->ptes[i]->block);
				delete app->ptes[i];
			}
		}
		else if(app->ptes[i]->file == ""){
			//resident or shared swap back pages
			if(app->ptes[i]->num_refs == 1){
				//no longer shared, write can be re-enabled.
				app->ptes[i]->pte.write_enable = (app->ptes[i]->reference && app->ptes[i]->resident && app->ptes[i]->dirty);
			}else if(app->ptes[i]->num_refs == 0){
				//"evict" no longer needed swap backed page from physmem and free swapblock.
				global_data.remove_from_clock(app->ptes[i]);
				global_data.free_swap_blocks.push(app->ptes[i]->block);
				delete app->ptes[i];
			}
		}
	}

	//delete app
	delete app;
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
	app_pt* app = global_data.app_map[global_data.curr_pid];
	
	//check if arena is full
	if(app->pte_next_index == VM_ARENA_SIZE/VM_PAGESIZE)
		return nullptr;

	if(!filename){
		//swap backed
		if(!global_data.reserve_blocks(0))
			return nullptr;
		app->reserve_blocks(1);

		//map to zero page
		return app->map_swap_backed();
	} else {
		unsigned int vpage = (unsigned int)(filename - (char*)VM_ARENA_BASEADDR)/VM_PAGESIZE;
		//maybe do arithmatic as char* and then case to unsigned int.
		unsigned int offset = (unsigned long long)filename - (vpage * VM_PAGESIZE);

		//check filename address is valid
		if (filename < VM_ARENA_BASEADDR || filename >= ((char*)VM_ARENA_BASEADDR + VM_ARENA_SIZE) || vpage >= app->pte_next_index)
			return nullptr;
			
		//copy filename from physmem to kernel mem.
		ostringstream file;
		for (int i = 0; !(filename < VM_ARENA_BASEADDR || filename >= ((char*)VM_ARENA_BASEADDR + VM_ARENA_SIZE) || vpage >= app->pte_next_index); ++i){
			//read fault if vpage is not read_enabled
			if(!app->ptes[vpage]->pte.read_enable){
				if(vm_fault((char*)VM_ARENA_BASEADDR + (vpage * VM_PAGESIZE), 0) == -1){
					return nullptr;
				}
			}
			//if end of filename, break from loop
			if (((char *)vm_physmem + (app->ptes[vpage]->pte.ppage * VM_PAGESIZE))[offset] == '\0') {
				break;
			}
			else {
				//add char to kernel string
				file << ((char *)vm_physmem + (app->ptes[vpage]->pte.ppage * VM_PAGESIZE))[offset];
				++offset;
				if (offset == VM_PAGESIZE) {
					//end of vpage, got to next one.
					offset = 0;
					++vpage;
				}
			}
		}
		//check if filename did not end before the end of the arena
		if (filename < VM_ARENA_BASEADDR || filename >= ((char*)VM_ARENA_BASEADDR + VM_ARENA_SIZE) || vpage >= app->pte_next_index)
			return nullptr;

		return app->map_file_backed(file.str(), block);
	}
}