#pragma once

#define GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK 512
#define GPU_MAX_MEMORY_ALLOCATIONS_PER_RING_FRAME 512

enum GPUResourceLifetime
{
	GPU_RESOURCE_LIFETIME_FRAME,
	GPU_RESOURCE_LIFETIME_PERSISTENT,
};

struct GPUMemoryAllocation
{
	GfxMemory memory;
	s64 offset;
	void *mappedPointer;
};

struct GPUSubbuffer
{
	GfxBuffer buffer;
	s64 *offset;
};

struct GPUImageAllocation
{
	GfxMemory memory;
	s64 *offset;
};

// A block allocator stores its memory in a linked list of fixed size blocks.
// Memory can then be suballocated out of each block, and when space in a block runs out, a new block is allocated from the render backend and added to the end of the linked list.
// Block allocators are dynamic and not thread-safe. All operations on a block allocator must lock the block allocator's mutex.

struct GPUMemoryBlock
{
	GfxMemory memory;
	void *mappedPointer;
	s64 frontier;
	s64 allocationCount;
	GPUMemoryAllocation allocations[GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK]; // @TODO: Use a dynamic array?
	GPUMemoryBlock *next;
};

struct GPUMemoryBlockAllocator
{
	Mutex mutex;
	s64 blockSize;
	GPUMemoryBlock *baseBlock;
	GPUMemoryBlock *activeBlock;
	GfxMemoryType memoryType;
};

struct GPUMemoryRingAllocator
{
	GfxMemory memory;
	GfxMemoryType memoryType;
	s64 capacity;
	s64 size;
	s64 frameSizes[GFX_MAX_FRAMES_IN_FLIGHT];
	s64 bottom, top;
	s64 allocationCounts[GFX_MAX_FRAMES_IN_FLIGHT];
	GPUMemoryAllocation allocations[GFX_MAX_FRAMES_IN_FLIGHT][GPU_MAX_MEMORY_ALLOCATIONS_PER_RING_FRAME];
	void *mappedPointer;
};

struct GPUIndexedGeometry
{
	GfxBuffer vertexBuffer;
	GfxBuffer indexBuffer;
};
