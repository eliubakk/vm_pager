#ifndef _VM_GLOBALS_H_
#define _VM_GLOBALS_H_

#include "app_pt.h"
#include <unordered_map>
#include <deque>
#include <queue>
#include <vector>

class vm_globals{
public:
	pid_t curr_pid;
	std::unordered_map<pid_t, app_pt*> app_map;
	size_t memory_pages;
	size_t max_swap_blocks;
	size_t swap_blocks_used;
	std::queue<unsigned int> free_swap_blocks;
	std::unordered_map<std::string, std::unordered_map<size_t, app_pt::app_pte*>> file_blocks;
	app_pt::app_pte* zero_page;
	std::deque<app_pt::app_pte*> clock;

	vm_globals();

	~vm_globals();

	bool reserve_blocks(pid_t parent);

	bool load_page(unsigned int vpage, char* buffer = nullptr);
};

extern vm_globals global_data;

#endif