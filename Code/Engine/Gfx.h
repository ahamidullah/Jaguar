#pragma once

struct GfxSubmitInfo
{
	Array<GfxCommandBuffer> commandBuffers;
	Array<GfxPipelineStageFlags> waitStages;
    Array<GfxSemaphore> waitSemaphores;
    Array<GfxSemaphore> signalSemaphores;
};

enum GfxCommandQueueType
{
	GFX_GRAPHICS_COMMAND_QUEUE,
	GFX_COMPUTE_COMMAND_QUEUE,
	GFX_TRANSFER_COMMAND_QUEUE,

	GFX_COMMAND_QUEUE_COUNT
};

enum GfxMemoryType
{
	GFX_GPU_ONLY_MEMORY,
	GFX_CPU_TO_GPU_MEMORY,
	GFX_GPU_TO_CPU_MEMORY,

	GFX_MEMORY_TYPE_COUNT,
};
