#pragma once

struct GfxQueueSubmitInfo
{
	Array<GfxCommandBuffer> commandBuffers;
    Array<GfxSemaphore> waitSemaphores;
    Array<GfxSemaphore> signalSemaphores;
};
