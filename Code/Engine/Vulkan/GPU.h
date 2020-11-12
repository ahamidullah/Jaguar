#pragma once

#ifdef VulkanBuild

#include "Instance.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "Swapchain.h"
#include "Buffer.h"
#include "StagingBuffer.h"
#include "CommandBuffer.h"
#include "Queue.h"
#include "Pipeline.h"

namespace GPU
{

typedef Vulkan::Buffer Buffer;
//typedef Vulkan::Material Material;
typedef Vulkan::StagingBuffer StagingBuffer;
typedef Vulkan::CommandBuffer CommandBuffer;
typedef Vulkan::Framebuffer Framebuffer;
typedef Vulkan::Shader Shader;
typedef Vulkan::QueueType QueueType;

struct GPU
{
	s64 frameIndex;
	Vulkan::Instance instance;
	Vulkan::PhysicalDevice physicalDevice;
	Vulkan::Device device;
	Vulkan::Swapchain swapchain;
	Vulkan::BufferAllocator bufferAllocator;
	Vulkan::CommandBufferPool commandBufferPool;
	Vulkan::Queues queues;
	VkPipelineLayout pipelineLayout;

	void BeginFrame();
	void EndFrame();
	//MeshAsset NewMeshAsset(MeshAsset *a);
	//Mesh NewMesh();
	Buffer NewVertexBuffer(s64 size);
	Buffer NewIndexBuffer(s64 size);
	//Material NewMaterial();
	StagingBuffer NewStagingBuffer();
	CommandBuffer NewCommandBuffer(QueueType t);
	Framebuffer NewFramebuffer();
	Framebuffer DefaultFramebuffer();
	Shader CompileShader(string::String filename, bool *err);
	void SubmitGraphicsCommands();
	void SubmitTransferCommands();
};

GPU New(Window *w);

}

#endif
