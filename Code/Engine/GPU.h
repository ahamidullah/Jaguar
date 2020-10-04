#pragma once

#include "Vulkan.h"

struct GPUIndexedGeometry
{
};

#if 0
#pragma once

#include "Vulkan.h"
#include "Gfx.h"

#include "Basic/Spinlock.h"

enum GPULifetime
{
	GPU_FRAME_LIFTIME,
	GPU_PERSISTENT_LIFETIME,
};

struct GPUMemory
{
	GPUInternalMemory internal;
	s64 offset;
	void *mapped;
};

struct GPUMemoryBlock
{
	GPUInternalMemory internal;
	void *mapped;
	s64 frontier;
	s64 allocationCount;
	Array<GPUMemoryAllocation> allocations;
	GPUMemoryBlock *next;
};

struct GPUMemoryBlockAllocator
{
	Spinlock lock;
	GPUMemoryBlock *activeBlock;

	s64 blockSize;
	GPUMemoryBlock *baseBlock;
	GPUMemoryType memoryType;
};

struct GPUMemoryRingAllocator
{
	s64 capacity;
	s64 size;
	StaticArray<s64, GPU_MAX_FRAMES_IN_FLIGHT> frameSizes;
	s64 start, end;
	StaticArray<s64, GPU_MAX_FRAMES_IN_FLIGHT> allocationCounts;
	StaticArray<Array<GPUMemoryAllocation>, GPU_MAX_FRAMES_IN_FLIGHT> allocations;
	GPUInternalMemory internal;
	GPUMemoryType memoryType;
	void *mapped;
};

struct GPUIndexedGeometry
{
	GfxBuffer vertexBuffer;
	GfxBuffer indexBuffer;
};

struct GPUImage
{
	GPUInternalImage internal;
	GPUMemory *memory;
};

struct GPUBuffer
{
	GPUInternalBuffer internal;
	GPUMemory *memory;
	s64 size;
};

struct GPUCommandBuffer
{
	GPUInternalCommandBuffer internal;
	GPUResourceLifetime lifetime;
	GPUCommandQueueType queueType;

	void BeginRenderPass(GPURenderPass rp, GPUFramebuffer fb);
	void EndRenderPass();
	void SetViewport(s64 w, s64 h);
	void SetScissor(u32 width, u32 height);
	void BindPipeline(GPUPipeline p);
	void BindVertexBuffer(GPUBuffer b);
	void BindIndexBuffer(GPUBuffer b);
	void BindDescriptorSets(GPUPipelineBindPoint pbp, GPUPipelineLayout pl, s64 firstSet, ArrayView<GPUDescriptorSet> dss);
	void DrawIndexedVertices(s64 numIndices, s64 firstIndex, s64 vertexOffset);
	void CopyBufferToImage(GPUBuffer b, GPUImage i, u32 w, u32 h);
	void QueueForSubmission();
};

void InitializeGPU();
GfxBuffer CreateGPUBuffer(s64 size, GfxBufferUsageFlags usage, GfxMemoryType memoryType, GPUResourceLifetime lifetime, void **mappedMemory);
GfxImage CreateGPUImage(s64 width, s64 height, GfxFormat format, GfxImageLayout initialLayout, GfxImageUsageFlags usage, GfxSampleCount sampleCount, GfxMemoryType memoryType, GPUResourceLifetime lifetime, void **mappedMemory = NULL);
GPUCommandBuffer CreateGPUCommandBuffer(GfxCommandQueueType queueType, GPUResourceLifetime lifetime);
void QueueGPUCommandBuffer(GPUCommandBuffer commandBuffer, bool *signalOnCompletion);
GfxSemaphore SubmitQueuedGPUCommandBuffers(GfxCommandQueueType queueType, Array<GfxSemaphore> frameWaitSemaphores, Array<GfxPipelineStageFlags> frameWaitStages, GfxFence frameFence);
void ClearGPUMemoryForFrameIndex(s64 frameIndex);
void ClearGPUCommandPoolsForFrameIndex(s64 frameIndex);
#endif
