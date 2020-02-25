#pragma once

namespace Renderer
{

#define GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK 512

struct GPUMemoryAllocation
{
	GPUMemory memory;
	u32 offset;
	void *mappedPointer;
};

struct GPUSubbuffer
{
	GPUBuffer buffer;
	u32 *offset;
};

struct GPUImageAllocation
{
	GPUMemory memory;
	u32 *offset;
};

// A block allocator stores its memory in a linked list of fixed size blocks.
// Memory can then be suballocated out of each block, and when space in a block runs out, a new block is allocated from the render backend and added to the end of the linked list.
// Block allocators are dynamic and not thread-safe. All operations on a block allocator must lock the block allocator's mutex.

struct GPUMemoryBlock
{
	GPUMemory memory;
	void *mappedPointer;
	u32 frontier;
	u32 allocationCount;
	GPUMemoryAllocation allocations[GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK];
	GPUMemoryBlock *next;
};

struct GPUMemoryBlockAllocator
{
	Mutex mutex;
	u32 blockSize;
	GPUMemoryBlock *baseBlock;
	GPUMemoryBlock *activeBlock;
	GPUMemoryType memoryType;
};

struct GPUMemoryRingAllocator
{
	GPUMemory memory;
	void *mappedPointer;
	u32 size;
	u32 readIndex;
	u32 writeIndex;
};

struct GPUIndexedGeometry
{
	GPUBuffer vertex_buffer;
	GPUBuffer index_buffer;
};

}
