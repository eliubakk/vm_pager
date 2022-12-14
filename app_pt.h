#ifndef _APP_PT_H_
#define _APP_PT_H_

#include "vm_pager.h"
#include <utility>
#include <string>

class app_pt{
public:
	struct app_pte{
		unsigned int num_refs = 0;
		std::string file;
		page_table_entry_t pte;
		unsigned int block;
		unsigned int dirty : 1;
		unsigned int resident : 1;
		unsigned int reference : 1;

		app_pte(std::string file_in, unsigned int block_in);
	};

	page_table_t *pt;
	size_t swap_blocks_used;
	unsigned int pte_next_index;
	app_pte* ptes[VM_ARENA_SIZE/VM_PAGESIZE];

	app_pt(pid_t parent);

	bool reserve_blocks(size_t reserve);
	void update_external_pt();

	void *map_swap_backed(int index = -1);
	void *map_file_backed(std::string filename, size_t block);

	~app_pt();
};

#endif