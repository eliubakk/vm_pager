#ifndef _APP_PT_H_
#define _APP_PT_H_

#include "vm_pager.h"
#include <bitset>
#include <utility>
#include <queue>

class app_pt{
public:
	struct app_pte{
		unsigned int num_refs = 0;
		char *file;
		page_table_entry_t pte;
		unsigned int block;
		unsigned int dirty : 1;
		unsigned int resident : 1;
		unsigned int reference : 1;

		app_pte(char* file_in, unsigned int block_in);
	};

	page_table_t *pt;
	size_t swap_blocks_used;
	std::queue<unsigned int> reserved_blocks;
	std::queue<unsigned int> used_blocks;
	unsigned int pte_next_index;
	app_pte* ptes[VM_ARENA_SIZE/VM_PAGESIZE];

	app_pt(pid_t parent);

	bool reserve_blocks(size_t reserve);

	void *map_swap_backed();

	~app_pt();
};

#endif