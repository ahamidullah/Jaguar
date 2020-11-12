#ifdef VulkanBuild

#include "GPU.h"
#include "Loader.h"

namespace GPU
{

GPU New(Window *w)
{
	auto gpu = GPU{};
	auto err = false;
	auto dll = OpenDLL("libvulkan.so", &err);
	if (err)
	{
		Abort("Vulkan", "Could not open Vulkan DLL libvulkan.so.");
	}
	Vulkan::LoadExportedFunctions(dll);
	Vulkan::LoadGlobalFunctions();
	auto instLayers = array::Array<const char *>{};
	if (DebugBuild)
	{
		instLayers.Append("VK_LAYER_KHRONOS_validation");
	}
	auto instExts = array::Make<const char *>(
		"VK_KHR_surface",
		"VK_KHR_get_physical_device_properties2");
	if (__linux__)
	{
		instExts.Append("VK_KHR_xcb_surface");
	}
	if (DebugBuild)
	{
		instExts.Append("VK_EXT_debug_utils");
	}
	else
	{
		Abort("Vulkan", "Unknown Vulkan surface instance extension: unsupported operating system.");
	}
	gpu.instance = Vulkan::NewInstance(instLayers, instExts);
	Vulkan::LoadInstanceFunctions(gpu.instance);
	gpu.physicalDevice = Vulkan::NewPhysicalDevice(gpu.instance, w);
	auto devExts = array::Make<const char *>(
		"VK_KHR_swapchain",
		"VK_EXT_memory_budget");
	gpu.device = Vulkan::NewDevice(gpu.physicalDevice, instLayers, devExts);
	Vulkan::LoadDeviceFunctions(gpu.device);
	gpu.swapchain = Vulkan::NewSwapchain(gpu.physicalDevice, gpu.device);
	gpu.commandBufferPool = Vulkan::NewCommandBufferPool(gpu.physicalDevice, gpu.device);
	gpu.bufferAllocator = Vulkan::NewBufferAllocator(gpu.physicalDevice, gpu.device);
	gpu.pipelineLayout = Vulkan::NewPipelineLayout(gpu.device);
	gpu.queues = Vulkan::NewQueues(gpu.physicalDevice, gpu.device);
	return gpu;
}

void GPU::BeginFrame()
{
	this->swapchain.AcquireNextImage(this->device, this->frameIndex);
	for (auto i = 0; i < s64(QueueType::Count); i += 1)
	{
		for (auto j = 0; j < WorkerThreadCount(); j += 1)
		{
			this->commandBufferPool.Release(QueueType(i), j, this->queues.submissions[vkCommandGroupFreeIndex][i][j]);
		}
	}
	this->commandBufferPool.ResetCommandPools(this->device, this->frameIndex);
	this->queues.ClearSubmissions(this->frameIndex);
}

void GPU::EndFrame()
{
	this->swapchain.Present(this->physicalDevice, this->queues, this->frameIndex);
	this->frameIndex = (this->frameIndex + 1) % Vulkan::MaxFramesInFlight;
	vkCommandGroupUseIndex = (vkCommandGroupUseIndex + 1) % (Vulkan::MaxFramesInFlight + 1);
	vkCommandGroupFreeIndex = (vkCommandGroupUseIndex + 1) % (Vulkan::MaxFramesInFlight + 1);
	//GPUEndFrame(); // @TODO
}

Buffer GPU::NewVertexBuffer(s64 size)
{
	return this->bufferAllocator.Allocate(this->physicalDevice, this->device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, size);
}

Buffer GPU::NewIndexBuffer(s64 size)
{
	return this->bufferAllocator.Allocate(this->physicalDevice, this->device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, size);
}

#if 0
	MeshAsset GPU::NewMeshAsset(Memory::Allocator *a, s64 vertCount, s64 vertSize, s64 indCount, s64 indSize, ArrayView<u32> submeshInds)
	{
		return Vulkan::NewMeshAsset(this->device, a, vertCount, vertSize, indCount, indSize, submeshInds);
	/*
		auto vb = GPU::NewBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertCount * vertSize);
		auto ib = GPU::NewBuffer(
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indCount * indSize);
		auto asset = MeshAsset
		{
			.firstIndex = (u32)(ib.offset / indSize),
			.vertexOffset = (s32)(vb.offset / vertSize),
			.submeshes = NewArrayIn<Submesh>(a, submeshInds.count),
			.vertexBuffer = vb,
			.indexBuffer = ib,
		};
			for (auto j = 0; j < cis[i].submeshIndices.count; j += 1)
			{
				out[i]->submeshes[j] = GPUSubmesh
				{
					.indexCount = cis[i].submeshIndices[j],
				};
			}
	*/
	}

	Mesh GPU::NewMesh(MeshAsset *a)
	{
		return Vulkan::NewMesh(this->device, a);
	/*
		return
		{
			.asset = a,
			.uniform = NewBuffer(
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				sizeof(MeshUniform)),
		};
	*/
	}
#endif

StagingBuffer GPU::NewStagingBuffer()
{
	return Vulkan::NewStagingBuffer(&this->physicalDevice, &this->device, &this->commandBufferPool, &this->bufferAllocator);
}

CommandBuffer GPU::NewCommandBuffer(QueueType t)
{
	return this->commandBufferPool.Get(this->device, t);
}

Framebuffer GPU::NewFramebuffer()
{
	return {};
}

Framebuffer GPU::DefaultFramebuffer()
{
	return Vulkan::DefaultFramebuffer(&this->swapchain, &this->device);
	//return this->framebufferAllocator.Default();
}

Shader GPU::CompileShader(String filename, bool *err)
{
	return Vulkan::CompileShader(this->device, this->physicalDevice.surfaceFormat, this->pipelineLayout, filename, err);
}

void GPU::SubmitGraphicsCommands()
{
	this->queues.SubmitGraphicsCommands(this->commandBufferPool.active[s64(QueueType::Graphics)], this->swapchain, this->frameIndex);
	this->commandBufferPool.ClearActive(QueueType::Graphics);
}

void GPU::SubmitTransferCommands()
{
	this->queues.SubmitTransferCommands(this->commandBufferPool.active[s64(QueueType::Transfer)], this->frameIndex);
	this->commandBufferPool.ClearActive(QueueType::Transfer);
}

}

#endif
