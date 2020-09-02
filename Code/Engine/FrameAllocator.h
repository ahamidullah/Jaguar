#pragma once

#if 0

enum FrameAllocatorTag
{
};

struct FrameAllocatorBlock
{
};

struct FrameAllocator
{
};

AllocatorInterface FrameAllocator(FrameAllocatorTag t);

HeapAllocator NewHeapAllocator(s64 blockSize, s64 blockCount, AllocatorInterface blockAlloc, AllocatorInterface arrayAlloc);
AllocatorInterface NewHeapAllocatorInterface(HeapAllocator *heap);
void *AllocateHeapMemory(void *heap, s64 size);
void *AllocateAlignedHeapMemory(void *heap, s64 size, s64 alignment);
void *ResizeHeapMemory(void *heap, void *memory, s64 size);
void FreeHeapMemory(void *heap, void *memory);
void ClearHeapAllocator(void *heap);
void FreeHeapAllocator(void *heap);

#endif
