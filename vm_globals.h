#ifndef _VM_GLOBALS_H_
#define _VM_GLOBALS_H_

#include "app_pt.h"
#include <unordered_map>
#include <deque>
#include <bitset>
#include <vector>

class vm_globals{
public:
	pid_t curr_pid;
	std::unordered_map<pid_t, app_pt*> app_map;
	size_t memory_pages;
	size_t max_swap_blocks;
	size_t swap_blocks_used;
	std::bitset<4096> swap_blocks;
	std::unordered_map<std::string, std::vector< app_pt::app_pte*>> file_blocks;
	std::deque<app_pt::app_pte*> clock;

	bool reserve_blocks(pid_t parent);

	void load_page(unsigned int vpage);
};

extern vm_globals global_data;

#endif