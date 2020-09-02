#if 0
// @TODO: Block pool.
// @TODO: Free unused blocks back to the block pool.
// @TODO: Different block sizes? Small, medium, large, etc.
// @TODO: False sharing?

#include "GPU.h"
#include "Job.h"
#include "Math.h"
#include "Render.h"

#include "Basic/Atomic.h"
#include "Basic/Log.h"

const auto GPU_MEMORY_BLOCK_SIZE = MegabytesToBytes(256);
const auto GPU_MEMORY_RING_SIZE = MegabytesToBytes(128);

struct GPUMemoryAllocators
{
	StaticArray<GPUMemoryBlockAllocator, GPU_MEMORY_TYPE_COUNT> block;
	GPUMemoryRingAllocator hostToDeviceRing;
};

struct ThreadLocalGPUState
{
	GPUInternalCommandPool frameCommandPools[GPU_MAX_FRAMES_IN_FLIGHT][GPU_COMMAND_QUEUE_COUNT];
	Array<GPUInternalCommandBuffer> queuedAsyncCommandBuffers[2][GPU_COMMAND_QUEUE_COUNT];
	Array<GPUInternalCommandBuffer> queuedFrameCommandBuffers[GPU_COMMAND_QUEUE_COUNT];
};

auto gpuMemoryAllocators = GPUMemoryAllocators{};
auto gpuCommandQueues = StaticArray<GfxCommandQueue, GPU_COMMAND_QUEUE_COUNT>{};
auto gpuAsyncCommandPools = StaticArray<GfxCommandPool, GPU_COMMAND_QUEUE_COUNT>{};
auto gpuAsyncCommandBufferArrayIndices = StaticArray<GPUCommandQueueCount, volatile s64>{};
auto gpuThreadLocal = Array<ThreadLocalGPUState>{};

enum GPUResourceType
{
	GPUBufferResource,
	GPUImageResource,
};

bool GPUMemoryTypeIsCPUVisible(GPUMemoryType t)
{
	if (t == GPU_HOST_TO_DEVICE_MEMORY || t == GPU_DEVICE_TO_HOST_MEMORY)
	{
		return true;
	}
	return false;
}

GPUMemoryRingAllocator NewGPUMemoryRingAllocator(GPUMemoryType t)
{
	auto a = GPUMemoryRingAllocator
	{
		.capacity = GPU_MEMORY_RING_SIZE,
		.memoryType = t,
	};
	if (!AllocateGPUBackendMemory(a.capacity, t, &a.memory))
	{
		Abort("GPU", "Failed to allocate ring allocator memory.");
	}
	if (GPUMemoryTypeIsCPUVisible(t))
	{
		a.mappedMemory = MapGPUMemory(a.memory, a.capacity, 0);
	}
	else
	{
		a.mappedMemory = NULL;
	}
	return a;
}

void LogGPUInitialization()
{
	LogInfo("GPU", "GPU info:\n");
	LogInfo("GPU", "	Heap indices:\n");
	LogInfo("GPU", "		GPU only: %d\n", GPUMemoryHeapIndex(GFX_GPU_ONLY_MEMORY));
	LogInfo("GPU", "		CPU-to-GPU: %d\n", GPUMemoryHeapIndex(GFX_CPU_TO_GPU_MEMORY));
	LogInfo("GPU", "		GPU-to-CPU: %d\n", GPUMemoryHeapIndex(GFX_GPU_TO_CPU_MEMORY));
	LogInfo("GPU", "	Memory budget:\n");
	LogInfo("GPU", "	Buffer-image granularity: %d\n", GetGPUBufferImageGranularity());
	PrintGPUMemoryInfo();
}

void InitializeGPU()
{
	auto minimumRequiredMemoryPerType = StaticArray<s64, GPUMemoryTypeCount>{};
	for (auto i = 0; i < GPUMemoryTypeCount; i++)
	{
		auto e = (GPUMemoryType)i;
		switch (e)
		{
		case GPUDeviceOnlyMemory:
		{
			minimumRequiredMemoryPerType[i] += GPU_MEMORY_BLOCK_SIZE;
		} break;
		case GFX_CPU_TO_GPU_MEMORY:
		{
			minimumRequiredMemoryPerType[i] += GPU_MEMORY_BLOCK_SIZE + GPU_MEMORY_RING_SIZE;
		} break;
		case GFX_GPU_TO_CPU_MEMORY:
		{
			minimumRequiredMemoryPerType[i] += MegabytesToBytes(64); // @TODO: Revisit this number once we start using this memory type...
		} break;
		default:
		{
			Abort("Unknown memory type %d.", i);
		} break;
		}
	}
	s64 minimumMemoryRequirementsPerHeap[GPU_MAX_MEMORY_HEAP_COUNT] = {};
	for (auto i = 0; i < GFX_MEMORY_TYPE_COUNT; i++)
	{
		minimumMemoryRequirementsPerHeap[GPUMemoryHeapIndex((GfxMemoryType)i)] += minimumRequiredMemoryPerType[i];
	}
	auto memoryInfo = GetGPUMemoryInfo();
	for (auto i = 0; i < memoryInfo.count; i++)
	{
		if (memoryInfo[i].budget < minimumMemoryRequirementsPerHeap[i])
		{
			Abort("Not enough GPU memory for heap %d: require at least %fmb, got %fmb.", i, BytesToMegabytes(minimumMemoryRequirementsPerHeap[i]), BytesToMegabytes(memoryInfo[i].budget));
		}
	}
	for (auto i = 0; i < GPUMemoryTypeCount; i++)
	{
		gpuGlobals.memoryAllocators.block[i].memoryType = (GfxMemoryType)i;
	}
	gpuGlobals.memoryAllocators.cpuToGPURing = CreateGPUMemoryRingAllocator(GFX_CPU_TO_GPU_MEMORY);
	for (auto i = 0; i < GFX_COMMAND_QUEUE_COUNT; i++)
	{
		gpuGlobals.asyncCommandPools[i] = GfxCreateCommandPool((GfxCommandQueueType)i);
	}
	ResizeArray(&gpuGlobals.threadLocal, GetWorkerThreadCount());
	for (auto &tl : gpuGlobals.threadLocal)
	{
		for (auto &cp : tl.frameCommandPools)
		{
			for (auto i = 0; i < GFX_COMMAND_QUEUE_COUNT; i++)
			{
				cp[i] = GfxCreateCommandPool((GfxCommandQueueType)i);
			}
		}
	}
	for (auto i = 0; i < GFX_COMMAND_QUEUE_COUNT; i++)
	{
		gpuGlobals.commandQueues[i] = GfxGetCommandQueue((GfxCommandQueueType)i);
	}
	LogGPUInitialization();
}

// @TODO: Buffer-image granularity.

GPUMemoryAllocation *AllocateFromGPUMemoryBlocks(GPUMemoryBlockAllocator *allocator, GfxMemoryRequirements memoryRequirements)
{
	LockMutex(&allocator->mutex);
	Defer(UnlockMutex(&allocator->mutex));

	Assert(memoryRequirements.size < GPU_MEMORY_BLOCK_SIZE);

	// @TODO: Try to allocate out of the freed allocations.
	if (!allocator->activeBlock || allocator->activeBlock->frontier + memoryRequirements.size > GPU_MEMORY_BLOCK_SIZE)
	{
		// Need to allocate a new block.
		auto newBlock = (GPUMemoryBlock *)malloc(sizeof(GPUMemoryBlock)); // @TODO
		if (!GfxAllocateMemory(GPU_MEMORY_BLOCK_SIZE, allocator->memoryType, &newBlock->memory))
		{
			Abort("Failed to allocate GPU memory for block allocator..."); // @TODO
			return NULL;
		}
		newBlock->frontier = 0;
		newBlock->allocationCount = 0;
		if (GPUMemoryTypeIsCPUVisible(allocator->memoryType))
		{
			newBlock->mappedMemory = GfxMapMemory(newBlock->memory, GPU_MEMORY_BLOCK_SIZE, 0);
		}
		else
		{
			newBlock->mappedMemory = NULL;
		}
		newBlock->next = NULL;
		if (allocator->baseBlock)
		{
			allocator->activeBlock->next = newBlock;
			allocator->activeBlock = newBlock;
		}
		else
		{
			allocator->baseBlock = newBlock;
			allocator->activeBlock = allocator->baseBlock;
		}
	}

	// Allocate out of the active block's frontier.
	auto newAllocation = &allocator->activeBlock->allocations[allocator->activeBlock->allocationCount++];
	auto allocationStartOffset = AlignTo(allocator->activeBlock->frontier, memoryRequirements.alignment);
	*newAllocation =
	{
		.memory = allocator->activeBlock->memory,
		.offset = allocationStartOffset,
	};
	if (GPUMemoryTypeIsCPUVisible(allocator->memoryType))
	{
		newAllocation->mappedMemory = (char *)allocator->activeBlock->mappedMemory + allocationStartOffset;
	} else
	{
		newAllocation->mappedMemory = NULL;
	}
	Assert(allocator->activeBlock->allocationCount < GPU_MAX_MEMORY_ALLOCATIONS_PER_BLOCK);
	allocator->activeBlock->frontier = allocationStartOffset + memoryRequirements.size;
	Assert(allocator->activeBlock->frontier <= GPU_MEMORY_BLOCK_SIZE);
	return newAllocation;
}

GPUMemoryAllocation *AllocateFromGPUMemoryRing(GPUMemoryRingAllocator *allocator, GfxMemoryRequirements memoryRequirements, s64 frameIndex)
{
	auto newEnd = 0, oldEnd = 0, allocationStart = 0, allocationSize = 0;
	do
	{
		oldEnd = allocator->end;
		auto alignmentOffset = AlignmentOffset(oldEnd, memoryRequirements.alignment);
		allocationSize = alignmentOffset + memoryRequirements.size;
		if (allocator->size + allocationSize > allocator->capacity)
		{
			// @TODO: Fallback to block allocators.
			Abort("Ring buffer ran out of space.\n");
		}
		allocationStart = (oldEnd + alignmentOffset) % allocator->capacity;
		newEnd = (allocationStart + memoryRequirements.size) % allocator->capacity;
	} while (AtomicCompareAndSwap(&allocator->end, oldEnd, newEnd) != oldEnd);

	AtomicFetchAndAdd(&allocator->size, allocationSize);
	AtomicFetchAndAdd(&allocator->frameSizes[frameIndex], allocationSize);

	auto allocationIndex = AtomicFetchAndAdd(&allocator->allocationCounts[frameIndex], 1);
	auto allocation = &allocator->allocations[frameIndex][allocationIndex];
	allocation->memory = allocator->memory;
	allocation->offset = allocationStart;
	if (GPUMemoryTypeIsCPUVisible(allocator->memoryType))
	{
		allocation->mappedMemory = (char *)allocator->mappedMemory + allocationStart;
	}
	return allocation;
}

GPUMemory *AllocateGPUMemory(GPUResourceType rt, GfxMemoryRequirements mr, GfxMemoryType mr, GPULifetime lt, void **memMapPtr)
{
	auto mem = (GPUMemory *){};
	if (lt == GPU_FRAME_LIFETIME && mt == GFX_CPU_TO_GPU_MEMORY)
	{
		mem = AllocateFromGPUMemoryRing(&gpuGlobals.memoryAllocators.ring, memoryRequirements, GetFrameIndex());
	}
	else
	{
		mem = AllocateFromGPUMemoryBlocks(&gpuGlobals.memoryAllocators.block[memoryType], memoryRequirements);
	}
	Assert(allocation);
	if (mappedMemory)
	{
		Assert(GPUMemoryTypeIsCPUVisible(memoryType));
		*mappedMemory = allocation->mappedMemory;
	}
	return allocation;
}

GPUBuffer CreateGPUBuffer(s64 size, GPUAPIBufferUsageFlags u, GPUAPIMemoryType mt, GPUResourceLifetime lt, void **memMapPtr)
{
	auto buf = NewGPUAPIBuffer(size, u);
	auto req = GetGPUAPIBufferMemoryRequirements(buf);
	auto mem = AllocateGPUMemory(GPU_BUFFER_RESOURCE, req, mt, lt, memMapPtr);
	BindGPUAPIBufferMemory(buf, mem->api, mem->offset);
	return buf;
}

GPUImage NewGPUImage(s64 w, s64 h, GPUFormat f, GPUImageLayout il, GPUImageUsageFlags uf, GPUSampleCount sc, GPUMemoryType mt, GPUResourceLifetime lt, void **mapped)
{
	auto img = GPUImage
	{
		.internal = NewGPUInternalImage(w, h, f, il, uf, sc),
	};
	auto req = GetGPUInternalImageMemoryRequirements(img.internal);
	img.memory = AllocateGPUMemory(GPU_IMAGE_RESOURCE, req, mt, lt, mapped);
	Assert(img.memory);
	BindGPUImageMemory(image, allocation->memory, allocation->offset);
	return image;
}

GPUCommandBuffer NewGPUCommandBuffer(GPUCommandQueueType qt, GPUResourceLifetime lt)
{
	auto cb = GPUCommandBuffer
	{
		.queueType = qt,
		.lifetime = lt,
	};
	if (lt == GPU_RESOURCE_LIFETIME_FRAME)
	{
		cb.internal = NewGPUInternalCommandBuffer(gpuThreadLocal[ThreadIndex()].frameCommandPools[FrameIndex()][qt]);
	}
	else
	{
		cb.internal = NewGPUInternalCommandBuffer(gpuAsyncCommandPools[qt]);
	}
	return cb;
}

void QueueGPUCommandBuffer(GPUCommandBuffer cb, bool *signalOnCompletion)
{
	EndGPUInternalCommandBuffer(cb.internal);
	if (cb.lifetime == GPU_FRAME_LIFETIME)
	{
		gpuThreadLocal[ThreadIndex()].queuedFrameCommandBuffers[cb.queueType].Append(cb.internal);
	}
	else
	{
		gpuThreadLocal[ThreadIndex()].queuedAsyncCommandBuffers[gpuAsyncCommandBufferArrayIndices[cb.queueType]][cb.queueType].Append(cb.internal);
	}
}

void SubmitQueuedAsyncGPUCommmandBuffers(GPUCommandQueueType qt)
{
	auto ai = gpuAsyncCommandBufferArrayIndices[qt];
	if (gpuAsyncCommandBufferArrayIndices[qt] == 0)
	{
		gpuAsyncCommandBufferArrayIndices[qt] = 1;
	}
	else
	{
		gpuAsyncCommandBufferArrayIndices[qt] = 0;
	}
	auto numCBs = 0;
	for (auto tl : gpuThreadLocal)
	{
		numCBs += tl.queuedAsyncCommandBuffers[ai][qt].count;
	}
	if (numCBs == 0)
	{
		return;
	}
	auto cbs = NewArrayWithCount<GPUInternalCommandBuffer>(ContextAllocator(), numCBs);
	Defer(cbs.Free());
	auto i = 0;
	for (auto &tl : gpuThreadLocal)
	{
		CopyMemory(&tl.queuedAsyncCommandBuffers[ai][qt][0], &cbs.elements[w], tl.queuedAsyncCommandBuffers[ai][qt].count * sizeof(GPUInternalCommandBuffer));
		i += tl.queuedAsyncCommandBuffers[ai][qt].count;
		tl.queuedAsyncCommandBuffers[ai][qt].Resize(0);
		// @TODO: Change capacity to some default value?
	}
	auto si = GfxSubmitInfo
	{
		.commandBuffers = cbs,
	};
	GfxSubmitCommandBuffers(gpuCommandQueues[qt], si, NULL);
}

GPUSemaphore SubmitQueuedFrameGPUCommandBuffers(GPUCommandQueueType qt, Array<GPUSemaphore> waitSemaphores, Array<GPUPipelineStageFlags> waitStages, GPUFence signalFence)
{
	auto numCBs = 0;
	for (auto tl : gpuThreadLocal)
	{
		numCBs += tl.queuedFrameCommandBuffers[qt].count;
	}
	Assert(numCBs != 0);
	auto cbs = NewArrayWithCount<GPUInternalCommandBuffer>(ContextAllocator(), numCBs);
	auto i = 0;
	for (auto tl : gpuThreadLocal)
	{
		CopyMemory(tl.queuedFrameCommandBuffers[qt].elements, &cbs.elements[i], tl.queuedFrameCommandBuffers[qt].count * sizeof(GPUBackendCommandBuffer));
		i += tl.queuedFrameCommandBuffers[queueType].count;
		tl.queuedFrameCommandBuffers[qt].Resize(0);
		// @TODO: Change capacity to some default value?
	}
	auto r = NewGPUSemaphore();
	auto si = GPUSubmitInfo
	{
		.commandBuffers = cbs,
		.waitStages = waitStages,
		.waitSemaphores = waitSemaphores,
		.signalSemaphores = NewStaticArray<GPUSemaphore>(r),
	};
	SubmitGPUInternalCommandBuffers(gpuCommandQueues[qt], si, signalFence);
	return r;
}

GPUSemaphore SubmitQueuedGPUCommandBuffers(GPUCommandQueueType qt, Array<GPUSemaphore> ss, Array<GPUPipelineStageFlags> psfs, GPUFence f)
{
	SubmitQueuedAsyncGPUCommmandBuffers(qt);
	return SubmitQueuedFrameGPUCommandBuffers(qt, ss, psfs, f);
}

void ClearGPUMemoryForFrameIndex(s64 frameIndex)
{
	auto Clear = [frameIndex](GPUMemoryRingAllocator *allocator)
	{
		allocator->start = (allocator->start + allocator->frameSizes[frameIndex]) % allocator->capacity;
		allocator->frameSizes[frameIndex] = 0;
		allocator->allocationCounts[frameIndex] = 0;
		allocator->size = 0;
	};
	Clear(&gpuGlobals.memoryAllocators.cpuToGPURing);
}

void ClearGPUCommandPoolsForFrameIndex(s64 frameIndex)
{
	for (auto &tl : gpuGlobals.threadLocal)
	{
		GfxResetCommandPool(tl.frameCommandPools[frameIndex][GFX_GRAPHICS_COMMAND_QUEUE]);
		GfxResetCommandPool(tl.frameCommandPools[frameIndex][GFX_TRANSFER_COMMAND_QUEUE]);
		GfxResetCommandPool(tl.frameCommandPools[frameIndex][GFX_COMPUTE_COMMAND_QUEUE]);
	}
}

#if 0
GfxSemaphore SubmitQueuedGPUTransferCommands()
{
	SwitchAtomicDoubleBuffer(&gpuGlobals.transferDoubleBuffer);

	auto semaphore = GfxCreateSemaphore();
	GfxSubmitInfo submitInfo =
	{
		.commandBuffers = CreateArray(gpuGlobals.transferDoubleBuffer.readBufferElementCount, gpuGlobals.transferDoubleBuffer.readBuffer->data),
		.signalSemaphores = CreateArray(1, &semaphore),
	};
	GfxSubmitCommandBuffers(GFX_TRANSFER_COMMAND_QUEUE, submitInfo, NULL);
	return semaphore;
}
#endif
#endif
