#pragma once

#define GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK 512

#include "vulkan.h"

struct GPU_Memory_Allocation {
	GPU_Memory memory;
	u32 offset;
	void *mapped_pointer;
};

struct GPU_Subbuffer {
	GPU_Buffer buffer;
	u32 *offset;
};

struct GPU_Image_Allocation {
	GPU_Memory memory;
	u32 *offset;
};

// A block allocator stores its memory in a linked list of fixed size blocks.
// Memory can then be suballocated out of each block, and when space in a block runs out, a new block is allocated from the render backend and added to the end of the linked list.
// Block allocators are dynamic and not thread-safe. All operations on a block allocator must lock the block allocator's mutex.

typedef struct GPU_Buffer_Memory_Block GPU_Buffer_Memory_Block;
typedef struct GPU_Buffer_Memory_Block GPU_Buffer_Memory_Block;
typedef struct GPU_Image_Memory_Block GPU_Image_Memory_Block;

struct GPU_Buffer_Memory_Block {
	GPU_Memory memory;
	GPU_Buffer buffer;
	u32 frontier; // @TODO
	void *mapped_pointer;
	u32 allocation_count;
	u32 allocations[GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK];
	GPU_Buffer_Memory_Block *next;
};

struct GPU_Image_Memory_Block {
	GPU_Memory memory;
	u32 frontier; // @TODO
	u32 allocation_count;
	u32 allocations[GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK];
	GPU_Image_Memory_Block *next;
};

struct GPU_Buffer_Block_Allocator {
	PlatformMutex mutex;
	GPU_Memory_Type memory_type;
	u32 block_size;
	GPU_Buffer_Usage_Flags buffer_usage_flags;
	GPU_Buffer_Memory_Block *base_block;
	GPU_Buffer_Memory_Block *active_block;
};

struct GPU_Image_Block_Allocator {
	PlatformMutex mutex;
	GPU_Memory_Type memory_type;
	u32 block_size;
	GPU_Image_Memory_Block *base_block;
	GPU_Image_Memory_Block *active_block;
};

struct GPU_Buffer_Ring_Allocator {
	GPU_Memory memory;
	GPU_Memory_Type memory_type;
	GPU_Buffer buffer;
	void *mapped_pointer;
	s32 size;
	s32 read_index;
	s32 write_index;
};

typedef struct GPU_Memory_Block GPU_Memory_Block;

struct GPU_Memory_Block {
	GPU_Memory memory;
	void *mapped_pointer;
	u32 frontier;
	u32 allocation_count;
	GPU_Memory_Allocation allocations[GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK];
	GPU_Memory_Block *next;
};

struct GPU_Memory_Block_Allocator {
	PlatformMutex mutex;
	u32 block_size;
	GPU_Memory_Block *base_block;
	GPU_Memory_Block *active_block;
	GPU_Memory_Type memory_type;
};

struct GPU_Memory_Ring_Allocator {
	GPU_Memory memory;
	void *mapped_pointer;
	u32 size;
	u32 read_index;
	u32 write_index;
};

struct GPU_Indexed_Geometry {
	GPU_Buffer vertex_buffer;
	GPU_Buffer index_buffer;
};

