#pragma once

enum GPUResourceLifetime
{
	GPU_RESOURCE_LIFETIME_FRAME,
	GPU_RESOURCE_LIFETIME_PERSISTENT,
};

struct GPUMemoryAllocation
{
	GfxMemory memory;
	s64 offset;
	void *mappedMemory;
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

#define GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK 512

struct GPUMemoryBlock
{
	GfxMemory memory;
	void *mappedMemory;
	s64 frontier;
	s64 allocationCount;
	GPUMemoryAllocation allocations[GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK]; // @TODO: Use a dynamic array?
	GPUMemoryBlock *next;
};

#define GPU_MEMORY_BLOCK_SIZE Megabyte(256)

struct GPUMemoryBlockAllocator
{
	Mutex mutex;
	GPUMemoryBlock *baseBlock;
	GPUMemoryBlock *activeBlock;
	GfxMemoryType memoryType;
};

#define GPU_MAX_MEMORY_ALLOCATIONS_PER_RING_FRAME 512

struct GPUMemoryRingAllocator
{
	s64 capacity;
	s64 size;
	s64 frameSizes[GFX_MAX_FRAMES_IN_FLIGHT];
	s64 start, end;
	s64 allocationCounts[GFX_MAX_FRAMES_IN_FLIGHT];
	GPUMemoryAllocation allocations[GFX_MAX_FRAMES_IN_FLIGHT][GPU_MAX_MEMORY_ALLOCATIONS_PER_RING_FRAME];
	GfxMemory memory;
	GfxMemoryType memoryType;
	void *mappedMemory;
};

struct GPUIndexedGeometry
{
	GfxBuffer vertexBuffer;
	GfxBuffer indexBuffer;
};

struct GPUBuffer
{
	GfxBuffer apiHandle;
	GPUMemoryAllocation *memory;
	s64 size;
};
