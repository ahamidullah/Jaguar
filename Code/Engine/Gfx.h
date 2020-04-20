#pragma once

struct GfxSubmitInfo
{
	Array<GfxCommandBuffer> commandBuffers;
    Array<GfxSemaphore> waitSemaphores;
    Array<GfxSemaphore> signalSemaphores;
};
