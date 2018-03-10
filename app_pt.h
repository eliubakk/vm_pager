#ifndef _APP_PT_H_
#define _APP_PT_H_

#include "vm_pager.h"

class app_pt{
public:
	struct app_pte{
		unsigned int num_refs = 0;
		char *file;
		unsigned int block;
		unsigned int dirty : 1;
		unsigned int resident : 1;
		unsigned int reference : 1;
	};

	page_table_t *pt;
	size_t swap_blocks;

	app_pte* ptes[VM_ARENA_SIZE/VM_PAGESIZE];

	app_pt(pid_t parent);

	~app_pt();
};

#endif