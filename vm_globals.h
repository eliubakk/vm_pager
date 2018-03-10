#ifndef _VM_GLOBALS_H_
#define _VM_GLOBALS_H_

#include "app_pt.h"
#include <unordered_map>
#include <deque>

class vm_globals{
public:
	pid_t curr_pid;
	std::unordered_map<pid_t, app_pt*> app_map;
	size_t memory_pages;
	size_t swap_blocks;
	size_t swap_blocks_used;
	std::deque<app_pt::app_pte> clock;

	bool reserve_blocks(pid_t parent);
};

extern vm_globals global_data;

#endif