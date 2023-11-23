#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"


// OUR-HELPER
void allocate_map_chunck_of_pages(uint32 start, uint32 end, enum ALLOCATOR_TYPE AT)
{
	for(uint32 va = start; va < end; va += PAGE_SIZE)
	{
		struct FrameInfo *ptr_frame_info;
		allocate_frame(&ptr_frame_info); // panic if not success
		map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE | PERM_PRESENT);// WHAT ABOUT PERM
		if (AT == PAGE_ALLOCATOR)
			is_page_filled[PDX(va)][PTX(va)] = start;
	}

	if (AT == PAGE_ALLOCATOR)
		is_page_filled[PDX(start)][PTX(start)] = start - end;
}

void deallocate_unmap_chunck_of_pages(uint32 start, uint32 end)
{
	for(uint32 va = start, *del = 0; va < end; va += PAGE_SIZE)
		unmap_frame(ptr_page_directory, va);
}

uint32 get_free_size(uint32 *va)
{
	uint32 *i = va;
	for (; !is_page_filled[PDX(i)][PTX(i)]; i += PAGE_SIZE);
	return i - va;
}

int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
    //TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
    //Initialize the dynamic allocator of kernel heap with the given start address, size & limit
    //All pages in the given range should be allocated
    //Remember: call the initialize_dynamic_allocator(..) to complete the initialization
    //Return:
    //    On success: 0
    //    Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

    //Comment the following line(s) before start coding...
    //panic("not implemented yet");
    //return 0;

	segment_break = start = ROUNDDOWN(daStart, PAGE_SIZE);
    initSizeToAllocate = ROUNDUP(initSizeToAllocate, PAGE_SIZE);

    if(daStart + initSizeToAllocate > daLimit)
        return E_NO_MEM;

    segment_break += initSizeToAllocate;
    allocate_map_chunck_of_pages(start, segment_break, PAGE_ALLOCATOR);

	hard_limit = daLimit;
	initialize_dynamic_allocator(start, initSizeToAllocate);


	return 0;
}

void* sbrk(int increment)
{
	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING====
	//return (void*)-1 ;
	//panic("not implemented yet");

	void* ret = (void*)segment_break;
	if (!increment)
		return ret;

	if (increment > 0)
	{
		if (segment_break + increment > hard_limit)
			panic("brk of block allocator can not exceed hard_limit of block allocator");
		uint32 begin = ROUNDUP(segment_break, PAGE_SIZE), end = ROUNDUP(segment_break + increment, PAGE_SIZE);
		allocate_map_chunck_of_pages(begin, end, PAGE_ALLOCATOR);
		segment_break = end;
	}
	else
	{
		uint32 begin = ROUNDUP(segment_break + increment, PAGE_SIZE), end = ROUNDUP(segment_break, PAGE_SIZE);
		deallocate_unmap_chunck_of_pages(begin, end);
		segment_break += increment;
		ret = (void*)segment_break;
	}

	return ret;
}


void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	//return NULL;
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		return alloc_block_FF(size);
	else
	{

		uint32 num_of_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE, free_pages = 0, max_free = 0, first_add = -1;
		for (int i = hard_limit + PAGE_SIZE; i < KERNEL_HEAP_MAX; i += PAGE_SIZE)
		{
			if (!is_page_filled[PDX(i)][PTX(i)])
			{
				if (first_add == -1)
					first_add = i;
				free_pages++;
				if (free_pages == num_of_pages)
					break;
			}
			else
			{
				free_pages = 0;
				first_add=-1;
			}
		}
		if (free_pages == num_of_pages && isKHeapPlacementStrategyFIRSTFIT())
		{

			for (int i = first_add; num_of_pages--; i += PAGE_SIZE)
			{
				if (!is_page_filled[PDX(i)][PTX(i)])
				{
					struct FrameInfo *ptr_frame_info;
					int ret = allocate_frame(&ptr_frame_info);

					if (!ret)
					{
						return NULL;
					}
					else
					{
						map_frame(ptr_page_directory, ptr_frame_info, i, PERM_PRESENT | PERM_WRITEABLE);// WHAT ABOUT PERM
					}
					is_page_filled[x][y] = first_add;
				}

			}
			is_page_filled[PDX(first_add)][PTX(first_add)] = -ROUNDUP(size, PAGE_SIZE);
			return (void *) first_add;
		}
	}
		return NULL;
}


void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	if((uint32)virtual_address >= start && (uint32)virtual_address < segment_break)
	{
		free_block(virtual_address);
	}
	else
	{
		uint32 va = (uint32)virtual_address;
		va = ROUNDDOWN(va, PAGE_SIZE);
		uint32 x = PDX(va), y = PTX(va);
		uint32 first_page = is_page_filled[x][y];
		if (is_page_filled[x][y] < 0)
			first_page = va;
		if(first_page == 0)
			return;

		for(uint32 i = first_page, c = (-is_page_filled[PDX(i)][PTX(i)]) / PAGE_SIZE; c--; i += PAGE_SIZE)
		{
			uint32 x = PDX(i), y = PTX(i);
			struct FrameInfo *del_frame;
			//if(is_page_filled[x][y] == first_page)
			{
				//cprintf("line 1");
				//tlb_invalidate(ptr_page_directory,(void *)i);
				uint32 *ptr=NULL;
				del_frame = get_frame_info(ptr_page_directory, i, &ptr);
				//free_frame(del_frame);
				unmap_frame(ptr_page_directory, i);
				is_page_filled[x][y] = 0;
			}
			//else
				//break;

		}
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	//change this "return" according to your answer
	uint32 va = to_frame_info(physical_address)->va;
	return va ? va + (physical_address & 0xFFF) : 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
    //TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
    //refer to the project presentation and documentation for details
    // Write your code here, remove the panic and write your code
    //panic("kheap_physical_address() is not implemented yet...!!");
    //change this "return" according to your answer

    uint32 x = PDX(virtual_address), y = PTX(virtual_address);

    uint32 *ptr_page_table;
    uint32 ret = get_page_table(ptr_page_directory, virtual_address, &ptr_page_table);
    if(ret == TABLE_NOT_EXIST)
        return 0;

    uint32 page_table_entry = ptr_page_table[y];
    if(ptr_page_table[y] == 0)
        return 0;

    uint32 frame_num = page_table_entry >> 12;
    uint32 offset = virtual_address % 4096;
    //get_frame_info()
    return (frame_num * 4096) + offset;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().


void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	//return NULL;
	//panic("krealloc() is not implemented yet...!!");

	if (!virtual_address)
			return kmalloc(new_size);

		if (!new_size)
			return kfree(virtual_address), NULL;

		uint8 is_BA_VA = (uint32)virtual_address >= start && (uint32)virtual_address < segment_break, is_small = new_size <= DYN_ALLOC_MAX_BLOCK_SIZE;
		if (is_BA_VA ^ is_small)
		{
			void* ret = kmalloc(new_size);
			if (ret)
				kfree(virtual_address);
			return ret;
		}
		if (is_BA_VA)
			return realloc_block_FF(virtual_address, new_size);


		uint32 prog_size = -is_page_filled[PDX(virtual_address)][PTX(virtual_address)], init_va_size = prog_size + get_free_size(virtual_address + prog_size);

		if (new_size <= init_va_size)
		{
			kfree(virtual_address);
			allocate_map_chunck_of_pages((uint32) virtual_address, (uint32) virtual_address + new_size, PAGE_ALLOCATOR);
			return virtual_address;
		}
		else
		{
			void* ret = kmalloc(new_size);
			if (ret)
				kfree(virtual_address);
			return ret;
		}
}
