#ifdef VulkanBuild

#include "Vulkan.h"
#include "ShaderGlobal.h"
#include "Render.h"
#include "Math.h"
#include "Basic/DLL.h"
#include "Basic/Array.h"
#include "Basic/Hash.h"
#include "Basic/HashTable.h"
#include "Basic/Log.h"
#include "Basic/Memory.h"
#include "Basic/Atomic.h"
#include "Basic/Pool.h"
#include "Common.h"

#define VulkanExportedFunction(name) PFN_##name name = NULL;
#define VulkanGlobalFunction(name) PFN_##name name = NULL;
#define VulkanInstanceFunction(name) PFN_##name name = NULL;
#define VulkanDeviceFunction(name) PFN_##name name = NULL;
	#include "VulkanFunction.h"
#undef VulkanExportedFunction
#undef VulkanGlobalFunction
#undef VulkanInstanceFunction
#undef VulkanDeviceFunction

// @TODO: Make sure that all failable Vulkan calls are VkCheck'd.
// @TODO: Do eg.
//        typedef VImageLayout GFX_Image_Layout;
//        #define GFX_IMAGE_LAYOUT_1 VK_IMAGE_LAYOUT_1
//        etc... That will save us from having to convert between the two values.
// @TODO: Some kind of memory protection for the Gfx buffer!
// @TODO: Better texture descriptor set update scheme.
// @TODO: Move any uniform data that's updated per-frame to shared memory.
// @TODO: Scene stuff should be pbr?
// @TODO: Shouldn't per-swapchain resources actually be per-frame?
// @TODO: Query available VRAM and allocate based on that.
// @TODO: Switch to dynamic memory segments for vulkan Gfx memory.
//        If more VRAM becomes available while the game is running, the game should use it!
// @TODO: Change 'model' to local? E.g. 'model_to_world_space' -> 'local_to_world_space'.
// @TODO: Avoid VK_SHARING_MODE_CONCURRENT? "Go for VK_SHARING_MODE_EXCLUSIVE and do explicit queue family ownership barriers."
// @TODO: Dedicated transfer queue.
// @TODO: Experiment with HOST_CACHED memory?
// @TODO: Keep memory mapped persistently.
// @TODO: Use a single allocation and sub-allocations for image memory.
// @TODO: Different memory management system for shared allocations.
//        Main vertex, index, image = dynamic allocator, device memory
//        Uniforms = fixed size with offsets, device memory
//        Staging = stack allocator, shared memory, per-stage lifetime
//        Per-frame memory (debug/gui vertices/indices/uvs) = stack allocator, shared memory, per-frame lifetime
//        (Pool of blocks for shared memory)
// @TODO: Gfx memory defragmenting.
// @TODO: Debug clear freed memory?
// @TODO: Debug memory protection?
// @TODO: Read image pixels and mesh verices/indices directly into Gfx accessable staging memory.
// @TODO: What happens if MAX_FRAMES_IN_FLIGHT is less than or greater than the number of swapchain images?

// @TODO: CrashHandler log device features and limits if initialization is done.

enum VulkanQueueType
{
	VulkanGraphicsQueue,
	VulkanTransferQueue,
	VulkanComputeQueue,
	VulkanMainQueueTypeCount,
	VulkanPresentQueue = VulkanMainQueueTypeCount,
	VulkanTotalQueueTypeCount
};

enum VulkanMemoryType
{
	VulkanGPUMemory,
	VulkanCPUToGPUMemory,
	VulkanGPUToCPUMemory,
	VulkanMemoryTypeCount
};

struct VulkanShader
{
	Array<VkShaderStageFlagBits> vkStages;
	Array<VkShaderModule> vkModules;
};

const auto VulkanMaxFramesInFlight = 2;
// We keep +1 of VulkanMaxFramesInFlight to handle the case of uses which come before we've actually gotten the fence signal confirming the frame's resources are cleared.
const auto VulkanMemoryFrameCount = VulkanMaxFramesInFlight + 1;

// A VulkanMemoryFrameAllocator is a single memory allocation, from which we allocate memory associated with a specific frame.
// Once that frame is finished, all of the allocations associated with that frame are cleared.
// If the allocator runs out of memory, an overflow allocator is used.
struct VulkanMemoryFrameAllocator
{
	Spinlock lock;
	s64 capacity;
	s64 available;
	StaticArray<s64, VulkanMemoryFrameCount> frameSizes;
	s64 start, end;
	VkDeviceMemory vkMemory;
	VkBuffer vkBuffer;
	VkBufferUsageFlags bufferUsage;
	VulkanMemoryType memoryType;
	void *map;

	GPUBuffer AllocateBuffer(s64 size, void **map);
	void FreeOldestFrame();
};

auto vkCPUStagingFrameAllocator = VulkanMemoryFrameAllocator{};
auto vkGPUIndirectFrameAllocator = VulkanMemoryFrameAllocator{};

struct VulkanMemoryBlock
{
	VkDeviceMemory vkMemory;
	VkBuffer vkBuffer;
	void *map;
	s64 frontier;
};

struct VulkanMemoryBlockAllocator
{
	Spinlock lock;
	s64 blockSize;
	Array<VulkanMemoryBlock> blocks;
	VulkanMemoryType memoryType;
	VkDeviceSize bufferMemorySize;
	VkBufferUsageFlags bufferUsage;

	GPUBuffer AllocateBuffer(s64 size, void **map);
};

auto vkGPUVertexBlockAllocator = VulkanMemoryBlockAllocator{};
auto vkGPUIndexBlockAllocator = VulkanMemoryBlockAllocator{};
auto vkGPUUniformBlockAllocator = VulkanMemoryBlockAllocator{};
auto vkCPUStagingBlockAllocator = VulkanMemoryBlockAllocator{};

struct VulkanAsyncStagingBufferResources
{
	VkBuffer vkBuffer;
};

// @TODO: Get rid of this?
struct VulkanThreadLocal
{
	// Async resources.
	// @TODO: Could use double buffers to avoid the locking.
	Spinlock asyncCommandBufferLock;
	StaticArray<VkCommandPool, VulkanMainQueueTypeCount> asyncCommandPools;
	StaticArray<Array<VkCommandBuffer>, VulkanMainQueueTypeCount> asyncQueuedCommandBuffers;
	StaticArray<Array<Array<VkCommandBuffer>>, VulkanMainQueueTypeCount> asyncPendingCommandBuffers;
	StaticArray<Array<bool *>, VulkanMainQueueTypeCount> asyncSignals;
	StaticArray<Array<GPUFence>, VulkanMainQueueTypeCount> asyncFences;
	Spinlock asyncStagingBufferLock;
	Array<bool> asyncStagingSignals;
	Array<VulkanAsyncStagingBufferResources> asyncPendingStagingBuffers;
};

struct VulkanFramebufferKey
{
	VkRenderPass renderPass;
	u64 framebufferID;

	bool operator==(VulkanFramebufferKey v)
	{
		return this->renderPass == v.renderPass && this->framebufferID == v.framebufferID;
	}
};

u64 HashVulkanFramebufferKey(VulkanFramebufferKey k)
{
	return HashPointer(k.renderPass) ^ Hash64(k.framebufferID);
}

auto vkFramebufferCache = NewHashTable<VulkanFramebufferKey, VkFramebuffer>(0, HashVulkanFramebufferKey);

auto vkThreadLocal = Array<VulkanThreadLocal>{};
auto vkDebugMessenger = VkDebugUtilsMessengerEXT{};
auto vkInstance = VkInstance{};
auto vkPhysicalDevice = VkPhysicalDevice{};
auto vkDevice = VkDevice{};
auto vkQueueFamilies = StaticArray<u32, VulkanTotalQueueTypeCount>{};
auto vkQueues = StaticArray<VkQueue, VulkanTotalQueueTypeCount>{};
auto vkChangeImageOwnershipFromGraphicsToPresentQueueCommands = Array<VkCommandBuffer>{};
auto vkPresentMode = VkPresentModeKHR{};
auto vkPresentCommandPool = VkCommandPool{};
// @TODO: Create one present command buffer which gets reused.
auto vkSurface = VkSurfaceKHR{};
auto vkSurfaceFormat = VkSurfaceFormatKHR{};
auto vkBufferImageGranularity = s64{};
auto vkMemoryHeapCount = s64{};
auto vkMemoryTypeToMemoryFlags = StaticArray<VkMemoryPropertyFlags, VulkanMemoryTypeCount>{};
auto vkMemoryTypeToMemoryIndex = StaticArray<u32, VulkanMemoryTypeCount>{};
auto vkMemoryTypeToHeapIndex = StaticArray<u32, VulkanMemoryTypeCount>{};
auto vkSwapchain = VkSwapchainKHR{};
auto vkSwapchainImageIndex = u32{}; // @TODO
auto vkSwapchainImages = Array<VkImage>{};
auto vkSwapchainImageViews = Array<VkImageView>{};
auto vkPipelineLayout = VkPipelineLayout{};
auto vkDescriptorPool = VkDescriptorPool{};
// @TODO: Convert some of these Arrays to StaticArrays?
auto vkDescriptorSetLayouts = StaticArray<VkDescriptorSetLayout, ShaderDescriptorSetCount>{}; 
auto vkDescriptorSets = Array<Array<VkDescriptorSet>>{};
auto vkDescriptorSetBuffers = Array<Array<GPUBuffer>>{};
auto vkDescriptorSetUpdateFence = VkFence{};
auto vkImageOwnershipSemaphores = StaticArray<VkSemaphore, VulkanMaxFramesInFlight>{};
auto vkImageAcquiredSemaphores = StaticArray<VkSemaphore, VulkanMaxFramesInFlight>{};
auto vkFrameGraphicsCompleteSemaphores = StaticArray<VkSemaphore, VulkanMaxFramesInFlight>{};
auto vkFrameTransfersCompleteSemaphores = StaticArray<VkSemaphore, VulkanMaxFramesInFlight>{};
auto vkFrameIndex = u64{};
// Some vulkan pooled frame resources (such as frame command pools) cannot exist per-frame because we need to be able to allocate for frame X before we wait on the frame fence.
// We can't free those resources on a per-frame basis because that would also free all resources allocated in this frame before the call to GPUBeginFrame.
// So, we keep track of NumberOfFrames + 1 number of resource groups
auto vkCommandGroupUseIndex = u64{}; // Extended frame resources are allocated to this index every frame.
auto vkCommandGroupFreeIndex = u64{1}; // Extended frame resources are freed from this index every frame.
auto vkFrameMemoryUseIndex = u64{};
auto vkFrameMemoryFreeIndex = u64{1};
auto vkFrameFences = StaticArray<VkFence, VulkanMaxFramesInFlight>{};
auto vkRenderPasses = StaticArray<VkRenderPass, GPUShaderIDCount>{};
auto vkPipelines = StaticArray<VkPipeline, GPUShaderIDCount>{};
auto vkPipelineCache = VkPipelineCache{};
auto vkShaders = StaticArray<VulkanShader, GPUShaderIDCount>{};
auto vkDefaultFramebufferAttachments = Array<Array<GPUImageView>>{};
auto vkDefaultDepthImage = VkImage{};
auto vkDefaultDepthImageView = VkImageView{};

const auto VulkanCommandGroupCount = VulkanMaxFramesInFlight + 1;

auto vkFrameCommandPools = StaticArray<StaticArray<Array<VkCommandPool>, VulkanMainQueueTypeCount>, VulkanCommandGroupCount>{};
auto vkFrameQueuedCommandBuffers = StaticArray<StaticArray<Array<Array<VkCommandBuffer>>, VulkanMainQueueTypeCount>, VulkanCommandGroupCount>{};
auto vkFrameCommandBufferPools = StaticArray<StaticArray<Array<Array<VkCommandBuffer>>, VulkanMainQueueTypeCount>, VulkanCommandGroupCount>{}; // @TODO: Initialize the pool with some command buffers on startup.

struct VulkanMeshInfo
{
	u32 indexCount;
	u32 instanceCount;
	u32 firstIndex;
	s32 vertexOffset;
};

auto vkMeshInfos = Array<VulkanMeshInfo>{};

struct VulkanMeshBuffers
{
	VkBuffer vkIndexBuffer;
	VkBuffer vkVertexBuffer;
};

struct VulkanMeshBufferOffsets
{
	s64 indexBufferOffset;
	s64 vertexBufferOffset;
};

auto vkMeshBuffers = Array<VulkanMeshBuffers>{};
auto vkMeshBufferOffsets = Array<VulkanMeshBufferOffsets>{};

//struct VulkanUniform
//{
//};

//auto vkGlobalUniform = VuklanUniform{};
//auto vkCameraUniforms = Array<VulkanUniform>{};
//auto vkMaterialUniforms = Array<VulkanUniform>{};
//auto vkMeshUniforms = Array<VulkanUniform>{};

// Different shader types?
// Don't-care buffers?

GPUMesh NewGPUMesh(s64 vertSize, s64 indSize)
{
	// @TODO: Locking.
	Assert(vkMeshInfos.count == vkMeshBuffers.count);
	auto ib = NewGPUIndexBuffer(indSize);
	auto vb = NewGPUVertexBuffer(vertSize);
#if BIG_GPU_MESH
	auto m = GPUMesh
	{
		.indexCount = (u32)(indSize / sizeof(u16)),
		.instanceCount = 1,
		.firstIndex = (u32)(ib.offset / sizeof(u16)),
		.vertexOffset = (s32)(vb.offset / sizeof(Vertex1P1N)), // @TODO
		.indexBuffer = ib,
		.vertexBuffer = vb,
	};
#else
	auto m = GPUMesh
	{
		.id = (u64)vkMeshInfos.count,
	};
	vkMeshInfos.Append(
	{
		.indexCount = (u32)(indSize / sizeof(u16)),
		.instanceCount = 1,
		.firstIndex = (u32)(ib.offset / sizeof(u16)),
		.vertexOffset = (s32)(vb.offset / sizeof(Vertex1P1N)), // @TODO
	});
	vkMeshBuffers.Append(
	{
		.vkIndexBuffer = ib.vkBuffer,
		.vkVertexBuffer = vb.vkBuffer,
	});
	vkMeshBufferOffsets.Append(
	{
		.indexBufferOffset = ib.offset,
		.vertexBufferOffset = vb.offset,
	});
#endif
	return m;
}

GPUBuffer GPUMesh::VertexBuffer()
{
#if BIG_GPU_MESH
	return this->vertexBuffer;
#else
	return
	{
		.vkBuffer = vkMeshBuffers[this->id].vkVertexBuffer,
		.offset = vkMeshBufferOffsets[this->id].vertexBufferOffset,
	};
#endif
}

GPUBuffer GPUMesh::IndexBuffer()
{
#if BIG_GPU_MESH
	return this->indexBuffer;
#else
	return
	{
		.vkBuffer = vkMeshBuffers[this->id].vkIndexBuffer,
		.offset = vkMeshBufferOffsets[this->id].indexBufferOffset,
	};
#endif
}

u64 HashMeshRenderGroupData(MeshRenderGroupData d)
{
	return HashPointer(d.vkVertexBuffer) ^ HashPointer(d.vkIndexBuffer);
}

bool MeshRenderGroupData::operator==(MeshRenderGroupData d)
{
	return this->vkVertexBuffer == d.vkVertexBuffer && this->vkIndexBuffer == d.vkIndexBuffer;
}

GPUMeshGroup NewGPUFrameMeshGroup(ArrayView<GPUMesh> ms)
{
#if BIG_GPU_MESH
#if 1
	#if 1
	auto t = NewTimer("MeshGroup");
	// @TODO: Multithread FrameMeshGroup generation.
	// @TODO: Sort the MeshRenderGroups to minimize state changes.
	const auto InitialGroupSize = 64;
	struct FillHashTableParameter
	{
		HashTable<MeshRenderGroupData, Array<s64>> *hashTable;
		ArrayView<GPUMesh> meshes;
	};
	auto FillHashTable = [](void *param)
	{
		auto p = (FillHashTableParameter *)param;
		for (auto i = 0; i < p->meshes.count; i += 1)
		{
			auto d = MeshRenderGroupData
			{
				.vkVertexBuffer = p->meshes[i].vertexBuffer.vkBuffer,
				.vkIndexBuffer = p->meshes[i].indexBuffer.vkBuffer,
			};
			if (auto is = p->hashTable->LookupPointer(d); is)
			{
				is->Append(i);
			}
			else
			{
				auto nis = NewArrayWithCapacity<s64>(p->meshes.count);
				nis.Append(i);
				p->hashTable->Insert(d, nis);
			}
		}
	};
	auto hts = NewArray<HashTable<MeshRenderGroupData, Array<s64>>>(WorkerThreadCount());
	for (auto &ht : hts)
	{
		ht = NewHashTable<MeshRenderGroupData, Array<s64>>(InitialGroupSize, HashMeshRenderGroupData);
	}
	{
		auto params = NewArray<FillHashTableParameter>(WorkerThreadCount());
		auto js = NewArray<JobDeclaration>(WorkerThreadCount());
		auto workSize = DivideAndRoundUp(ms.count, WorkerThreadCount());
		for (auto i = 0; i < WorkerThreadCount(); i += 1)
		{
			auto workStart = i * workSize;
			if (workStart > ms.count)
			{
				workStart -= workStart - ms.count;
			}
			auto workEnd = (i + 1) * workSize;
			if (workEnd > ms.count)
			{
				workEnd -= workEnd - ms.count;
			}
			params[i] = FillHashTableParameter
			{
				.hashTable = &hts[i],
				.meshes = ms.View(workStart, workEnd),
			};
			js[i] = NewJobDeclaration(FillHashTable, &params[i]);
		}
		auto c = (JobCounter *){};
		RunJobs(js, NormalJobPriority, &c);
		c->Wait();
	}
	auto combinedHT = NewHashTable<MeshRenderGroupData, Array<s64>>(InitialGroupSize, HashMeshRenderGroupData);
	for (auto ht : hts)
	{
		for (auto e : ht)
		{
			if (auto is = combinedHT.LookupPointer(e.key); is)
			{
				is->AppendAll(e.value);
			}
			else
			{
				auto nis = NewArrayWithCapacity<s64>(meshes.count);
				nis.AppendAll(e.value);
				combinedHT.Insert(e.key, nis);
			}
		}
	}
	struct MakeGroupParameter
	{
		ArrayView<s64> meshIndices;
		ArrayView<GPUMesh> meshes;
		MeshRenderGroupData data;
		MeshRenderGroup *group;
	};
	auto MakeGroup = [](void *param)
	{
		auto p = (MakeGroupParameter *)param;
		auto cmds = NewGPUFrameIndirectBuffer(p->meshIndices.count * sizeof(VkDrawIndexedIndirectCommand));
		auto s = NewGPUFrameStagingBufferX(p->meshIndices.count * sizeof(VkDrawIndexedIndirectCommand), cmds);
		auto b = NewArrayView((VkDrawIndexedIndirectCommand *)s.Map(), p->meshIndices.count);
		for (auto i = 0; i < p->meshIndices.count; i += 1)
		{
			auto mi = p->meshIndices[i];
			b[i] = VkDrawIndexedIndirectCommand
			{
				.indexCount = p->meshes[mi].indexCount,
				.instanceCount = p->meshes[mi].instanceCount,
				.firstIndex = p->meshes[mi].firstIndex,
				.vertexOffset = p->meshes[mi].vertexOffset,
			};
		}
		s.Flush();
		*p->group = MeshRenderGroup
		{
			.data = p->data,
			.commands = cmds,
			.commandCount = p->meshIndices.count,
		};
	};
	auto gs = NewArray<MeshRenderGroup>(combinedHT.count);
	{
		auto params = NewArray<MakeGroupParameter>(combinedHT.count);
		auto js = NewArray<JobDeclaration>(combinedHT.count);
		auto i = 0;
		for (auto e : combinedHT)
		{
			params[i] = MakeGroupParameter
			{
				.meshIndices = e.value,
				.meshes = ms,
				.data = e.key,
				.group = &gs[i],
			};
			js[i] = NewJobDeclaration(MakeGroup, &params[i]);
			i += 1;
		}
		auto c = (JobCounter *){};
		RunJobs(js, NormalJobPriority, &c);
		c->Wait();
	}
	return
	{
		.renderGroups = gs,
	};
	#endif
	#if 0
	auto ht = NewHashTable<MeshRenderGroupData, Array<s64>>(128, HashMeshRenderGroupData);
	for (auto i = 0; i < ms.count; i += 1)
	{
		auto d = MeshRenderGroupData
		{
			.vkVertexBuffer = ms[i].vertexBuffer.vkBuffer,
			.vkIndexBuffer = ms[i].indexBuffer.vkBuffer,
		};
		if (auto is = ht.LookupPointer(d); is)
		{
			is->Append(i);
		}
		else
		{
			auto a = NewArrayWithCapacity<s64>(ms.count);
			a.Append(i);
			ht.Insert(d, a);
		}
	}
	auto gs = Array<MeshRenderGroup>{};
	for (auto e : ht)
	{
		auto cmds = NewGPUFrameIndirectBuffer(e.value.count * sizeof(VkDrawIndexedIndirectCommand));
		auto s = NewGPUFrameStagingBufferX(e.value.count * sizeof(VkDrawIndexedIndirectCommand), cmds);
		auto b = NewArrayView((VkDrawIndexedIndirectCommand *)s.Map(), e.value.count);
		for (auto i = 0; i < e.value.count; i += 1)
		{
			auto mi = e.value[i];
			b[i] = VkDrawIndexedIndirectCommand
			{
				.indexCount = ms[mi].indexCount,
				.instanceCount = ms[mi].instanceCount,
				.firstIndex = ms[mi].firstIndex,
				.vertexOffset = ms[mi].vertexOffset,
			};
		}
		s.Flush();
		gs.Append(
			MeshRenderGroup
			{
				.data = e.key,
				.commands = cmds,
				.commandCount = e.value.count,
			});
	}
	return
	{
		.renderGroups = gs,
	};
	#endif
	#if 0
		auto inserted = false;
		for (auto j = 0; j < ds.count; j += 1)
		{
			if (ds[j].vkVertexBuffer == ms[i].vertexBuffer.vkBuffer && ds[j].vkIndexBuffer == ms[i].indexBuffer.vkBuffer)
			{
				groupIndices[j].Append(i);
				inserted = true;
				break;
			}
			cmps += 1;
		}
		if (!inserted)
		{
			ds.Append(
				MeshRenderGroupData
				{
					.vkVertexBuffer = ms[i].vertexBuffer.vkBuffer,
					.vkIndexBuffer = ms[i].indexBuffer.vkBuffer,
				});
			auto a = NewArrayWithCapacity<s64>(ms.count);
			a.Append(i);
			groupIndices.Append(a);
		}
	}
	ConsolePrint("cmps per frame %d.\n", cmps);
	auto groups = Array<MeshRenderGroup>{};
	for (auto i = 0; i < groupIndices.count; i += 1)
	{
		auto cmds = NewGPUFrameIndirectBuffer(groupIndices[i].count * sizeof(VkDrawIndexedIndirectCommand));
		auto s = NewGPUFrameStagingBufferX(groupIndices[i].count * sizeof(VkDrawIndexedIndirectCommand), cmds);
		auto b = NewArrayView((VkDrawIndexedIndirectCommand *)s.Map(), groupIndices[i].count);
		for (auto j = 0; j < groupIndices[i].count; j += 1)
		{
			auto mi = groupIndices[i][j];
			b[j] = VkDrawIndexedIndirectCommand
			{
				.indexCount = ms[mi].indexCount,
				.instanceCount = ms[mi].instanceCount,
				.firstIndex = ms[mi].firstIndex,
				.vertexOffset = ms[mi].vertexOffset,
			};
		}
		s.Flush();
		groups.Append(
			MeshRenderGroup
			{
				.data = ds[i],
				.commands = cmds,
				.commandCount = groupIndices[i].count,
			});
	}
	return
	{
		.renderGroups = groups,
	};
	#endif
#else
	auto ib = NewGPUFrameIndirectBuffer(ms.count * sizeof(VkDrawIndexedIndirectCommand));
	auto s = NewGPUFrameStagingBufferX(ms.count * sizeof(VkDrawIndexedIndirectCommand), ib);
	auto b = NewArrayView((VkDrawIndexedIndirectCommand *)s.Map(), ms.count);
	for (auto i = 0; i < ms.count; i += 1)
	{
		b[i] = VkDrawIndexedIndirectCommand
		{
			.indexCount = ms[i].indexCount,
			.instanceCount = ms[i].instanceCount,
			.firstIndex = ms[i].firstIndex,
			.vertexOffset = ms[i].vertexOffset,
		};
	}
	s.Flush();
	return
	{
		.indirectBuffer = ib,
	};
#endif
#else
	auto ib = NewGPUFrameIndirectBuffer(ms.count * sizeof(VkDrawIndexedIndirectCommand));
	auto s = NewGPUFrameStagingBufferX(ms.count * sizeof(VkDrawIndexedIndirectCommand), ib);
	auto b = NewArrayView((VkDrawIndexedIndirectCommand *)s.Map(), ms.count);
	for (auto i = 0; i < ms.count; i += 1)
	{
		auto mi = &vkMeshInfos[ms[i].id];
		b[i] = VkDrawIndexedIndirectCommand
		{
			.indexCount = mi->indexCount,
			.instanceCount = mi->instanceCount,
			.firstIndex = mi->firstIndex,
			.vertexOffset = mi->vertexOffset,
		};
	}
	s.Flush();
	return
	{
		.indirectBuffer = ib,
	};
#endif
}

void GPUCommandBuffer::DrawMeshes(GPUMeshGroup mg, ArrayView<GPUMesh> ms)
{
#if BIG_GPU_MESH
#if 1
	for (auto rg : mg.renderGroups)
	{
		vkCmdBindIndexBuffer(this->vkCommandBuffer, rg.data.vkIndexBuffer, 0, GPUIndexTypeUint16);
		auto offset = (VkDeviceSize)0;
		vkCmdBindVertexBuffers(this->vkCommandBuffer, 0, 1, &rg.data.vkVertexBuffer, &offset);
		//this->BindIndexBuffer(mb->indexBuffer, GPUIndexTypeUint16);
		//this->BindVertexBuffer(mb->vertexBuffer, 0);
		//this->DrawIndexed(mi->indexCount, 0, 0);
		this->DrawIndexedIndirect(rg.commands, rg.commandCount);
	}
#else
	vkCmdBindIndexBuffer(this->vkCommandBuffer, ms[0].indexBuffer.vkBuffer, 0, GPUIndexTypeUint16);
	auto offset = (VkDeviceSize)0;
	vkCmdBindVertexBuffers(this->vkCommandBuffer, 0, 1, &ms[0].vertexBuffer.vkBuffer, &offset);
	//this->BindIndexBuffer(mb->indexBuffer, GPUIndexTypeUint16);
	//this->BindVertexBuffer(mb->vertexBuffer, 0);
	//this->DrawIndexed(mi->indexCount, 0, 0);
	this->DrawIndexedIndirect(mg.indirectBuffer, ms.count);
#endif
#else
	auto mb = &vkMeshBuffers[0];
	//auto mi = &vkMeshInfos[m.id];
	vkCmdBindIndexBuffer(this->vkCommandBuffer, mb->vkIndexBuffer, 0, GPUIndexTypeUint16);
	auto offset = (VkDeviceSize)0;
	vkCmdBindVertexBuffers(this->vkCommandBuffer, 0, 1, &mb->vkVertexBuffer, &offset);
	//this->BindIndexBuffer(mb->indexBuffer, GPUIndexTypeUint16);
	//this->BindVertexBuffer(mb->vertexBuffer, 0);
	//this->DrawIndexed(mi->indexCount, 0, 0);
	this->DrawIndexedIndirect(mg.indirectBuffer, ms.count);
#endif
}

#if DebugBuild
	// https://developer.nvidia.com/blog/vulkan-dos-donts/
	// Aim for 15-30 command buffers and 5-10 vkQueueSubmit() calls per frame...
	auto vkNumberOfCommandBuffersUsedThisFrame = 0;
	auto vkNumberOfQueueSubmitsThisFrame = 0;
#endif

#define VkCheck(x)\
	do { \
		VkResult _r = (x); \
		if (_r != VK_SUCCESS) Abort("Vulkan", "VkCheck failed on '%s': %k", #x, VkResultToString(_r)); \
	} while (0)

String VkResultToString(VkResult r)
{
	switch (r)
	{
		case (VK_SUCCESS):
			return "VK_SUCCESS";
		case (VK_NOT_READY):
			return "VK_NOT_READY";
		case (VK_TIMEOUT):
			return "VK_TIMEOUT";
		case (VK_EVENT_SET):
			return "VK_EVENT_SET";
		case (VK_EVENT_RESET):
			return "VK_EVENT_RESET";
		case (VK_INCOMPLETE):
			return "VK_INCOMPLETE";
		case (VK_ERROR_OUT_OF_HOST_MEMORY):
			return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case (VK_ERROR_INITIALIZATION_FAILED):
			return "VK_ERROR_INITIALIZATION_FAILED";
		case (VK_ERROR_DEVICE_LOST):
			return "VK_ERROR_DEVICE_LOST";
		case (VK_ERROR_MEMORY_MAP_FAILED):
			return "VK_ERROR_MEMORY_MAP_FAILED";
		case (VK_ERROR_LAYER_NOT_PRESENT):
			return "VK_ERROR_LAYER_NOT_PRESENT";
		case (VK_ERROR_EXTENSION_NOT_PRESENT):
			return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case (VK_ERROR_FEATURE_NOT_PRESENT):
			return "VK_ERROR_FEATURE_NOT_PRESENT";
		case (VK_ERROR_INCOMPATIBLE_DRIVER):
			return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case (VK_ERROR_TOO_MANY_OBJECTS):
			return "VK_ERROR_TOO_MANY_OBJECTS";
		case (VK_ERROR_FORMAT_NOT_SUPPORTED):
			return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case (VK_ERROR_FRAGMENTED_POOL):
			return "VK_ERROR_FRAGMENTED_POOL";
		case (VK_ERROR_OUT_OF_DEVICE_MEMORY):
			return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case (VK_ERROR_OUT_OF_POOL_MEMORY):
			return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case (VK_ERROR_INVALID_EXTERNAL_HANDLE):
			return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case (VK_ERROR_SURFACE_LOST_KHR):
			return "VK_ERROR_SURFACE_LOST_KHR";
		case (VK_ERROR_NATIVE_WINDOW_IN_USE_KHR):
			return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case (VK_SUBOPTIMAL_KHR):
			return "VK_SUBOPTIMAL_KHR";
		case (VK_ERROR_OUT_OF_DATE_KHR):
			return "VK_ERROR_OUT_OF_DATE_KHR";
		case (VK_ERROR_INCOMPATIBLE_DISPLAY_KHR):
			return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case (VK_ERROR_VALIDATION_FAILED_EXT):
			return "VK_ERROR_VALIDATION_FAILED_EXT";
		case (VK_ERROR_INVALID_SHADER_NV):
			return "VK_ERROR_INVALID_SHADER_NV";
		case (VK_ERROR_NOT_PERMITTED_EXT):
			return "VK_ERROR_NOT_PERMITTED_EXT";
		case (VK_RESULT_MAX_ENUM):
			return "VK_RESULT_MAX_ENUM";
		case (VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT):
			return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		case (VK_ERROR_FRAGMENTATION_EXT):
			return "VK_ERROR_FRAGMENTATION_EXT";
		case (VK_ERROR_INVALID_DEVICE_ADDRESS_EXT):
			return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
		case (VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT):
			return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		case (VK_ERROR_INCOMPATIBLE_VERSION_KHR):
			return "VK_ERROR_INCOMPATIBLE_VERSION_KHR";
		case (VK_ERROR_UNKNOWN):
			return "VK_ERROR_UNKNOWN";
		case (VK_THREAD_IDLE_KHR):
			return "VK_THREAD_IDLE_KHR";
		case (VK_THREAD_DONE_KHR):
			return "VK_THREAD_DONE_KHR";
		case (VK_OPERATION_DEFERRED_KHR):
			return "VK_OPERATION_DEFERRED_KHR";
		case (VK_OPERATION_NOT_DEFERRED_KHR):
			return "VK_OPERATION_NOT_DEFERRED_KHR";
		case (VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT):
			return "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT";
	}
	return "UnknownVkResultCode";
}

String GPUShaderName(GPUShaderID id)
{
	switch (id)
	{
	case GPUModelShaderID:
	{
		return "Model";
	} break;
	case GPUShaderIDCount:
	default:
	{
		LogError("Vulkan", "Invalid shader id %d.", id);
		return "";
	};
	}
	return "";
}

u32 VulkanDebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT sev, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *data, void *userData)
{
	auto log = LogLevel{};
	auto sevStr = String{};
	switch (sev)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	{
		log = VerboseLog;
		sevStr = "Verbose";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
	{
		log = InfoLog;
		sevStr = "Info";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
	{
		log = ErrorLog;
		sevStr = "Warning";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
	{
		log = ErrorLog;
		sevStr = "Error";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
	default:
	{
		log = ErrorLog;
		sevStr = "Unknown";
	};
	}
	auto typeStr = String{};
	switch (type)
	{
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
	{
		typeStr = "General";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
	{
		typeStr = "Validation";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
	{
		typeStr = "Performance";
	} break;
	default:
	{
		typeStr = "Unknown";
	};
	}
	if (sevStr == "Error" && typeStr == "Validation")
	{
		Abort("Vulkan", "%k: %k: %s", sevStr, typeStr, data->pMessage);
	}
	LogPrint(log, "Vulkan", "%k: %k: %s", sevStr, typeStr, data->pMessage);
    return 0;
}

GPUMemoryHeapInfo GPUMemoryUsage()
{
	auto bp = VkPhysicalDeviceMemoryBudgetPropertiesEXT
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT
	};
	auto mp = VkPhysicalDeviceMemoryProperties2
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
		.pNext = &bp,
	};
	vkGetPhysicalDeviceMemoryProperties2(vkPhysicalDevice, &mp);
	return
	{
		.usage = bp.heapUsage[vkMemoryTypeToHeapIndex[VulkanGPUMemory]],
		.budget = bp.heapBudget[vkMemoryTypeToHeapIndex[VulkanGPUMemory]],
	};
}

void LogGPUMemoryInfo()
{
	auto bp = VkPhysicalDeviceMemoryBudgetPropertiesEXT
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT
	};
	auto mp = VkPhysicalDeviceMemoryProperties2
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
		.pNext = &bp,
	};
	vkGetPhysicalDeviceMemoryProperties2(vkPhysicalDevice, &mp);
	LogInfo("Vulkan", "Memory info:");
	for (auto i = 0; i < vkMemoryHeapCount; i++)
	{
		LogInfo(
			"Vulkan",
			"	Heap: %d, Usage: %fmb, Total: %fmb",
			i,
			BytesToMegabytes(bp.heapUsage[i]),
			BytesToMegabytes(bp.heapBudget[i]));
	}
}

bool IsVulkanMemoryTypeCPUVisible(VulkanMemoryType t)
{
	if (t == VulkanCPUToGPUMemory || t == VulkanGPUToCPUMemory)
	{
		return true;
	}
	return false;
}

VkBuffer NewVulkanBuffer(VkBufferUsageFlags u, s64 size);

VulkanMemoryFrameAllocator NewVulkanMemoryFrameAllocator(VulkanMemoryType mt, VkBufferUsageFlags bu, s64 cap)
{
	auto b = NewVulkanBuffer(bu, cap);
	auto mr = VkMemoryRequirements{};
	vkGetBufferMemoryRequirements(vkDevice, b, &mr);
	auto ai = VkMemoryAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = (VkDeviceSize)mr.size,
		.memoryTypeIndex = vkMemoryTypeToMemoryIndex[mt],
	};
	auto mem = VkDeviceMemory{};
	auto rc = vkAllocateMemory(vkDevice, &ai, NULL, &mem);
	if (rc == VK_ERROR_OUT_OF_DEVICE_MEMORY || rc == VK_ERROR_OUT_OF_HOST_MEMORY)
	{
		Abort("Vulkan", "Failed to allocate memory for frame allocator.");
	}
	VkCheck(rc);
	VkCheck(vkBindBufferMemory(vkDevice, b, mem, 0));
	auto a = VulkanMemoryFrameAllocator
	{
		.capacity = cap,
		.available = cap,
		.vkMemory = mem,
		.vkBuffer = b,
		.bufferUsage = bu,
	};
	if (IsVulkanMemoryTypeCPUVisible(mt))
	{
		VkCheck(vkMapMemory(vkDevice, a.vkMemory, 0, cap, 0, &a.map));
	}
	else
	{
		a.map = NULL;
	}
	return a;
}

GPUBuffer VulkanMemoryFrameAllocator::AllocateBuffer(s64 size, void **map)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	this->available -= size;
	if (this->available < 0)
	{
		// @TODO: Fallback to block allocators.
		Abort("Vulkan", "Frame allocator ran out of space...");
	}
	this->frameSizes[vkFrameMemoryUseIndex] += size;
	auto b = GPUBuffer
	{
		.vkBuffer = this->vkBuffer,
	};
	if (this->end + size > this->capacity)
	{
		b.offset = 0;
		this->end = size;
	}
	else
	{
		b.offset = this->end;
		this->end += size;
	}
	if (map)
	{
		*map = (u8 *)this->map + b.offset;
	}
	return b;
}

void VulkanMemoryFrameAllocator::FreeOldestFrame()
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	this->start = (this->start + this->frameSizes[vkFrameMemoryFreeIndex]) % this->capacity;
	this->available += this->frameSizes[vkFrameMemoryFreeIndex];
	this->frameSizes[vkFrameMemoryFreeIndex] = 0;
}

VulkanMemoryBlockAllocator NewVulkanMemoryBlockAllocator(VulkanMemoryType mt, VkBufferUsageFlags bu, s64 blockSize)
{
	// Dummy buffer to retrieve the size and alignment requirements of our memory allocation.
	auto mr = VkMemoryRequirements{};
	auto b = NewVulkanBuffer(bu, blockSize);
	vkGetBufferMemoryRequirements(vkDevice, b, &mr);
	vkDestroyBuffer(vkDevice, b, NULL);
	return
	{
		.blockSize = blockSize,
		.blocks = NewArrayIn<VulkanMemoryBlock>(GlobalAllocator(), 0),
		.memoryType = mt,
		.bufferMemorySize = mr.size,
		.bufferUsage = bu,
	};
}

GPUBuffer VulkanMemoryBlockAllocator::AllocateBuffer(s64 size, void **map)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	Assert(size < this->blockSize);
	if (this->blocks.count == 0 || this->blocks.Last()->frontier + size > this->blockSize)
	{
		auto newBlk = VulkanMemoryBlock{};
		newBlk.vkBuffer = NewVulkanBuffer(this->bufferUsage, this->blockSize);
		auto ai = VkMemoryAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = (VkDeviceSize)this->bufferMemorySize,
			.memoryTypeIndex = vkMemoryTypeToMemoryIndex[this->memoryType],
		};
		auto rc = vkAllocateMemory(vkDevice, &ai, NULL, &newBlk.vkMemory);
		if (rc == VK_ERROR_OUT_OF_DEVICE_MEMORY || rc == VK_ERROR_OUT_OF_HOST_MEMORY)
		{
			Abort("Vulkan", "Failed to allocate memory for block allocator."); // @TODO
		}
		VkCheck(rc);
		VkCheck(vkBindBufferMemory(vkDevice, newBlk.vkBuffer, newBlk.vkMemory, 0));
		newBlk.frontier = 0;
		if (IsVulkanMemoryTypeCPUVisible(this->memoryType))
		{
			VkCheck(vkMapMemory(vkDevice, newBlk.vkMemory, 0, this->blockSize, 0, &newBlk.map));
		}
		else
		{
			newBlk.map = NULL;
		}
		this->blocks.Append(newBlk);
	}
	auto blk = this->blocks.Last();
	auto b = GPUBuffer
	{
		.vkBuffer = blk->vkBuffer,
		.offset = blk->frontier,
	};
	blk->frontier += size;
	if (map)
	{
		*map = ((u8 *)blk->map) + b.offset;
	}
	Assert(blk->frontier <= this->blockSize);
	return b;
}

#ifdef __linux__
	const char *RequiredVulkanSurfaceInstanceExtension()
	{
		return "VK_KHR_xcb_surface";
	}

	xcb_connection_t *XCBConnection();

	VkResult NewVulkanSurface(Window *w, VkInstance inst, VkSurfaceKHR *s)
	{
		auto ci = VkXcbSurfaceCreateInfoKHR
		{
			.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
			.connection = XCBConnection(),
			.window = w->xcbWindow,
		};
		return vkCreateXcbSurfaceKHR(inst, &ci, NULL, s);
	}
#endif

const auto VulkanInitialDescriptorSetBufferSize = MegabytesToBytes(4);
const auto VulkanDepthBufferFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
const auto VulkanDepthBufferInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
const auto VulkanDepthBufferImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
const auto VulkanDepthBufferSampleCount = VK_SAMPLE_COUNT_1_BIT;

void InitializeGPU(Window *win)
{
	// @TODO:
	//PushContextAllocator(FrameAllocator("Vulkan"));
	//Defer(PopContextAllocator());
	auto err = false;
	auto lib = OpenDLL("libvulkan.so", &err);
	if (err)
	{
		Abort("Vulkan", "Could not open Vulkan DLL libvulkan.so.");
	}
	#define VulkanExportedFunction(name) \
		name = (PFN_##name)lib.Lookup(#name, &err); \
		if (err) Abort("Vulkan", "Failed to load Vulkan function %s: Vulkan version 1.1 required.", #name);
	#define VulkanGlobalFunction(name) \
		name = (PFN_##name)vkGetInstanceProcAddr(NULL, (const char *)#name); \
		if (!name) Abort("Failed to load Vulkan function %s: Vulkan version 1.1 required", #name);
	#define VulkanInstanceFunction(name)
	#define VulkanDeviceFunction(name)
		#include "VulkanFunction.h"
	#undef VulkanExportedFunction
	#undef VulkanGlobalFunction
	#undef VulkanInstanceFunction
	#undef VulkanDeviceFunction
	auto reqDevExts = MakeStaticArray<const char *>(
		"VK_KHR_swapchain",
		"VK_EXT_descriptor_indexing",
		"VK_EXT_memory_budget");
	auto reqInstLayers = Array<const char *>{};
	if (DebugBuild)
	{
		reqInstLayers.Append("VK_LAYER_KHRONOS_validation");
	}
	auto reqInstExts = MakeArray<const char *>(
		RequiredVulkanSurfaceInstanceExtension(),
		"VK_KHR_surface",
		"VK_KHR_get_physical_device_properties2");
	if (DebugBuild)
	{
		reqInstExts.Append("VK_EXT_debug_utils");
	}
	{
		auto version = u32{};
		vkEnumerateInstanceVersion(&version);
		if (VK_VERSION_MAJOR(version) < 1 || (VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) < 1))
		{
			Abort("Vulkan", "Vulkan version 1.1.0 or greater required: version %d.%d.%d is installed", VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));
		}
		LogInfo("Vulkan", "Using Vulkan version %d.%d.%d", VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));
	}
	auto dbgInfo = VkDebugUtilsMessengerCreateInfoEXT
	{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = VulkanDebugMessageCallback,
	};
	// Create instance.
	{
		auto numInstLayers = u32{};
		vkEnumerateInstanceLayerProperties(&numInstLayers, NULL);
		auto instLayers = NewArray<VkLayerProperties>(numInstLayers);
		vkEnumerateInstanceLayerProperties(&numInstLayers, &instLayers[0]);
		LogVerbose("Vulkan", "Vulkan layers:");
		for (auto il : instLayers)
		{
			LogVerbose("Vulkan", "\t%s", il.layerName);
		}
		auto numInstExts = u32{0};
		VkCheck(vkEnumerateInstanceExtensionProperties(NULL, &numInstExts, NULL));
		auto instExts = NewArray<VkExtensionProperties>(numInstExts);
		VkCheck(vkEnumerateInstanceExtensionProperties(NULL, &numInstExts, instExts.elements));
		LogVerbose("Vulkan", "Available Vulkan instance extensions:");
		for (auto ie : instExts)
		{
			LogVerbose("Vulkan", "\t%s", ie.extensionName);
		}
		auto ai = VkApplicationInfo
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Jaguar",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "Jaguar",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_MAKE_VERSION(1, 2, 0),
		};
		// @TODO: Check that all required extensions/layers are available.
		auto ci = VkInstanceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &ai,
			.enabledLayerCount = (u32)reqInstLayers.count,
			.ppEnabledLayerNames = reqInstLayers.elements,
			.enabledExtensionCount = (u32)reqInstExts.count,
			.ppEnabledExtensionNames = reqInstExts.elements,
		};
		if (DebugBuild)
		{
			ci.pNext = &dbgInfo;
		}
		VkCheck(vkCreateInstance(&ci, NULL, &vkInstance));
	}
	#define VulkanExportedFunction(name)
	#define VulkanGlobalFunction(name)
	#define VulkanInstanceFunction(name) \
		name = (PFN_##name)vkGetInstanceProcAddr(vkInstance, (const char *)#name); \
		if (!name) Abort("Vulkan", "Failed to load Vulkan function %s: Vulkan version 1.1 required", #name);
	#define VulkanDeviceFunction(name)
		#include "VulkanFunction.h"
	#undef VulkanExportedFunction
	#undef VulkanGlobalFunction
	#undef VulkanInstanceFunction
	#undef VulkanDeviceFunction
	VkCheck(vkCreateDebugUtilsMessengerEXT(vkInstance, &dbgInfo, NULL, &vkDebugMessenger));
	VkCheck(NewVulkanSurface(win, vkInstance, &vkSurface));
	// Select physical device.
	// @TODO: Rank physical device and select the best one?
	{
		auto foundSuitablePhysDev = false;
		auto numPhysDevs = u32{};
		VkCheck(vkEnumeratePhysicalDevices(vkInstance, &numPhysDevs, NULL));
		if (numPhysDevs == 0)
		{
			Abort("Vulkan", "Could not find any graphics devices.");
		}
		LogVerbose("Vulkan", "Available graphics device count: %d.", numPhysDevs);
		auto physDevs = NewArray<VkPhysicalDevice>(numPhysDevs);
		VkCheck(vkEnumeratePhysicalDevices(vkInstance, &numPhysDevs, &physDevs[0]));
		if (DebugBuild)
		{
			LogVerbose("Vulkan", "Available graphics devices:");
			for (auto pd : physDevs)
			{
				auto dp = VkPhysicalDeviceProperties{};
				vkGetPhysicalDeviceProperties(pd, &dp);
				LogVerbose("Vulkan", "\t%s", dp.deviceName);
			}
		}
		for (auto pd : physDevs)
		{
			if (DebugBuild)
			{
				auto dp = VkPhysicalDeviceProperties{};
				vkGetPhysicalDeviceProperties(pd, &dp);
				LogVerbose("Vulkan", "Considering graphics device %s.", dp.deviceName);
			}
			auto df = VkPhysicalDeviceFeatures{};
			vkGetPhysicalDeviceFeatures(pd, &df);
			if (!df.samplerAnisotropy)
			{
				LogVerbose("Vulkan", "Skipping graphics device: missing device feature samplerAnisotropy.");
				continue;
			}
			if (!df.shaderSampledImageArrayDynamicIndexing)
			{
				LogVerbose("Vulkan", "Skipping graphics device: missing device feature shaderSampledImageArrayDynamicIndexing.");
				continue;
			}
			if (!df.multiDrawIndirect)
			{
				LogVerbose("Vulkan", "Skipping graphics device: missing device feature multiDrawIndirect.");
				continue;
			}
			auto numDevExts = u32{};
			VkCheck(vkEnumerateDeviceExtensionProperties(pd, NULL, &numDevExts, NULL));
			auto devExts = NewArray<VkExtensionProperties>(numDevExts);
			VkCheck(vkEnumerateDeviceExtensionProperties(pd, NULL, &numDevExts, devExts.elements));
			auto missingDevExt = (const char *){};
			for (auto rde : reqDevExts)
			{
				missingDevExt = rde;
				for (auto de : devExts)
				{
					if (CStringsEqual(de.extensionName, rde))
					{
						missingDevExt = NULL;
						break;
					}
				}
				if (missingDevExt)
				{
					break;
				}
			}
			if (missingDevExt)
			{
				LogVerbose("Vulkan", "Skipping graphics device: missing device extension %s.", missingDevExt);
				continue;
			}
			// Make sure the swap chain is compatible with our window surface.
			// If we have at least one supported surface format and present mode, we will consider the device.
			auto numSurfFmts = u32{};
			VkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(pd, vkSurface, &numSurfFmts, NULL));
			auto numPresentModes = u32{};
			VkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(pd, vkSurface, &numPresentModes, NULL));
			if (numSurfFmts == 0 || numPresentModes == 0)
			{
				continue;
			}
			// Select the best swap chain settings.
			auto surfFmts = NewArray<VkSurfaceFormatKHR>(numSurfFmts);
			VkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(pd, vkSurface, &numSurfFmts, &surfFmts[0]));
			auto surfFmt = surfFmts[0];
			if (numSurfFmts == 1 && surfFmts[0].format == VK_FORMAT_UNDEFINED)
			{
				// No preferred format, so we get to pick our own.
				surfFmt = VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
			}
			else
			{
				for (auto sf : surfFmts)
				{
					if (sf.format == VK_FORMAT_B8G8R8A8_UNORM && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					{
						surfFmt = sf;
						break;
					}
				}
			}
			// - VK_PRESENT_MODE_IMMEDIATE_KHR is for applications that don't care about tearing, or have some way of synchronizing their rendering with the display.
			// - VK_PRESENT_MODE_MAILBOX_KHR may be useful for applications that generally render a new presentable image every refresh cycle, but are occasionally early.
			//   In this case, the application wants the new image to be displayed instead of the previously-queued-for-presentation image that has not yet been displayed.
			// - VK_PRESENT_MODE_FIFO_KHR specifies that the presentation engine waits for the next vertical blanking period to update the current image. REQUIRED to be supported.
			//   This is for applications that don't want tearing ever. It's difficult to say how fast they may be, whether they care about stuttering/latency.
			// - VK_PRESENT_MODE_FIFO_RELAXED_KHR is for applications that generally render a new presentable image every refresh cycle, but are occasionally late.
			//   In this case (perhaps because of stuttering/latency concerns), the application wants the late image to be immediately displayed, even though that may mean some tearing.
			auto presentMode = VK_PRESENT_MODE_FIFO_KHR;
			auto pms = NewArray<VkPresentModeKHR>(numPresentModes);
			VkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(pd, vkSurface, &numPresentModes, pms.elements));
			for (auto pm : pms)
			{
				// @TODO: If vsync...
				if (pm == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					presentMode = pm;
					break;
				}
				// @TODO: If not vsync... first of VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR
			}
			presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			auto numQueueFams = u32{};
			vkGetPhysicalDeviceQueueFamilyProperties(pd, &numQueueFams, NULL);
			auto queueFams = NewArray<VkQueueFamilyProperties>(numQueueFams);
			vkGetPhysicalDeviceQueueFamilyProperties(pd, &numQueueFams, &queueFams[0]);
			auto graphicsQueueFam = -1;
			auto presentQueueFam = -1;
			auto computeQueueFam = -1;
			auto dedicatedTransferQueueFam = -1;
			for (auto i = 0; i < numQueueFams; i++)
			{
				if (queueFams[i].queueCount == 0)
				{
					continue;
				}
				if (queueFams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					graphicsQueueFam = i;
				}
				if (queueFams[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					computeQueueFam = i;
				}
				if ((queueFams[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && !(queueFams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
				{
					dedicatedTransferQueueFam = i;
				}
				auto ps = u32{};
				VkCheck(vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, vkSurface, &ps));
				if (ps)
				{
					presentQueueFam = i;
				}
			}
			if (graphicsQueueFam != -1 && presentQueueFam != -1 && computeQueueFam != -1)
			{
				vkPhysicalDevice = pd;
				vkQueueFamilies[VulkanGraphicsQueue] = graphicsQueueFam;
				// The graphics queue family can always be used as the transfer queue family, but if
				// we can find a dedicated transfer queue family, use that instead.
				if (dedicatedTransferQueueFam != -1)
				{
					vkQueueFamilies[VulkanTransferQueue] = dedicatedTransferQueueFam;
				}
				else
				{
					vkQueueFamilies[VulkanTransferQueue] = graphicsQueueFam;
				}
				vkQueueFamilies[VulkanComputeQueue] = computeQueueFam;
				vkQueueFamilies[VulkanPresentQueue] = presentQueueFam;
				vkSurfaceFormat = surfFmt;
				vkPresentMode = presentMode;
				foundSuitablePhysDev = true;
				LogVerbose("Vulkan", "Vulkan device extensions:");
				for (auto de : devExts)
				{
					LogVerbose("Vulkan", "\t%s", de.extensionName);
				}
				break;
			}
		}
		if (!foundSuitablePhysDev)
		{
			Abort("Vulkan", "Could not find suitable graphics device.");
		}
	}
	// Physical device info.
	{
		for (auto i = 0; i < VulkanMemoryTypeCount; i++)
		{
			auto e = (VulkanMemoryType)i;
			switch (e)
			{
			case VulkanGPUMemory:
			{
				vkMemoryTypeToMemoryFlags[i] = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			} break;
			case VulkanCPUToGPUMemory:
			{
				vkMemoryTypeToMemoryFlags[i] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			} break;
			case VulkanGPUToCPUMemory:
			{
				vkMemoryTypeToMemoryFlags[i] =
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
					| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
					| VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			} break;
			case VulkanMemoryTypeCount:
			default:
			{
				Abort("Vulkan", "Unknown memory type %d.", e);
			} break;
			}
		}
		auto devProps = VkPhysicalDeviceProperties{};
		vkGetPhysicalDeviceProperties(vkPhysicalDevice, &devProps);
		LogInfo("Vulkan", "Selected graphics device %s.", devProps.deviceName);
		vkBufferImageGranularity = devProps.limits.bufferImageGranularity;
		auto memProps = VkPhysicalDeviceMemoryProperties{};
		vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProps);
		vkMemoryHeapCount = memProps.memoryHeapCount;
		for (auto i = 0; i < VulkanMemoryTypeCount; i++)
		{
			auto found = false;
			for (auto j = 0; j < memProps.memoryTypeCount; j++)
			{
				auto memFlags = vkMemoryTypeToMemoryFlags[i];
				if ((memProps.memoryTypes[j].propertyFlags & memFlags) == memFlags)
				{
					vkMemoryTypeToMemoryIndex[i] = j;
					vkMemoryTypeToHeapIndex[i] = memProps.memoryTypes[j].heapIndex;
					found = true;
					break;
				}
			}
			if (!found)
			{
				Abort("Vulkan", "Unable to find GPU memory index and heap index for memory type %d.", i);
			}
		}
	}
	// Create logical device.
	{
		auto queuePrio = 1.0f;
		auto queueCIs = Array<VkDeviceQueueCreateInfo>{};
		queueCIs.Append(
			VkDeviceQueueCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = vkQueueFamilies[VulkanGraphicsQueue],
				.queueCount = 1,
				.pQueuePriorities = &queuePrio,
			});
		if (vkQueueFamilies[VulkanPresentQueue] != vkQueueFamilies[VulkanGraphicsQueue])
		{
			queueCIs.Append(
				VkDeviceQueueCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueFamilyIndex = vkQueueFamilies[VulkanPresentQueue],
					.queueCount = 1,
					.pQueuePriorities = &queuePrio,
				});
		}
		if (vkQueueFamilies[VulkanTransferQueue] != vkQueueFamilies[VulkanGraphicsQueue])
		{
			queueCIs.Append(
				VkDeviceQueueCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueFamilyIndex = vkQueueFamilies[VulkanTransferQueue],
					.queueCount = 1,
					.pQueuePriorities = &queuePrio,
				});
		}
		auto pdf = VkPhysicalDeviceFeatures
		{
			.multiDrawIndirect = VK_TRUE,
			.samplerAnisotropy = VK_TRUE,
		};
		auto pddif = VkPhysicalDeviceDescriptorIndexingFeaturesEXT
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
			.runtimeDescriptorArray = VK_TRUE,
		};
		auto ci = VkDeviceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &pddif,
			.queueCreateInfoCount = (u32)queueCIs.count,
			.pQueueCreateInfos = queueCIs.elements,
			.enabledLayerCount = (u32)reqInstLayers.count,
			.ppEnabledLayerNames = reqInstLayers.elements,
			.enabledExtensionCount = (u32)reqDevExts.Count(),
			.ppEnabledExtensionNames = reqDevExts.elements,
			.pEnabledFeatures = &pdf,
		};
		VkCheck(vkCreateDevice(vkPhysicalDevice, &ci, NULL, &vkDevice));
	}
#define VulkanExportedFunction(name)
#define VulkanGlobalFunction(name)
#define VulkanInstanceFunction(name)
#define VulkanDeviceFunction(name) \
	name = (PFN_##name)vkGetDeviceProcAddr(vkDevice, (const char *)#name); \
	if (!name) Abort("Vulkan", "Failed to load Vulkan function %s: Vulkan version 1.1 is required.", #name);
	#include "VulkanFunction.h"
#undef VulkanExportedFunction
#undef VulkanGlobalFunction
#undef VulkanInstanceFunction
#undef VulkanDeviceFunction
	//LogVulkanInitialization();
	// Create command queues.
	for (auto i = 0; i < VulkanTotalQueueTypeCount; i += 1)
	{
		vkGetDeviceQueue(vkDevice, vkQueueFamilies[i], 0, &vkQueues[i]); // No return.
	}
	// Create the presentation semaphores.
	{
		auto ci = VkSemaphoreCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		if (vkQueueFamilies[VulkanPresentQueue] != vkQueueFamilies[VulkanGraphicsQueue])
		{
			for (auto i = 0; i < VulkanMaxFramesInFlight; i++)
			{
				VkCheck(vkCreateSemaphore(vkDevice, &ci, NULL, &vkImageOwnershipSemaphores[i]));
			}
		}
		for (auto i = 0; i < VulkanMaxFramesInFlight; i++)
		{
			VkCheck(vkCreateSemaphore(vkDevice, &ci, NULL, &vkImageAcquiredSemaphores[i]));
			VkCheck(vkCreateSemaphore(vkDevice, &ci, NULL, &vkFrameGraphicsCompleteSemaphores[i]));
			VkCheck(vkCreateSemaphore(vkDevice, &ci, NULL, &vkFrameTransfersCompleteSemaphores[i]));
		}
	}
	// Initialize memory allocators.
	vkGPUVertexBlockAllocator = NewVulkanMemoryBlockAllocator(VulkanGPUMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MegabytesToBytes(1));
	vkGPUIndexBlockAllocator = NewVulkanMemoryBlockAllocator(VulkanGPUMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MegabytesToBytes(1));
	vkGPUUniformBlockAllocator = NewVulkanMemoryBlockAllocator(VulkanGPUMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MegabytesToBytes(64));
	vkCPUStagingBlockAllocator = NewVulkanMemoryBlockAllocator(VulkanCPUToGPUMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, MegabytesToBytes(256));
	vkCPUStagingFrameAllocator = NewVulkanMemoryFrameAllocator(VulkanCPUToGPUMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, MegabytesToBytes(256));
	vkGPUIndirectFrameAllocator = NewVulkanMemoryFrameAllocator(VulkanGPUMemory, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MegabytesToBytes(64));
	vkThreadLocal.Resize(WorkerThreadCount());
	// Create command groups.
	{
		auto aci = VkCommandPoolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		};
		for (auto &tl : vkThreadLocal)
		{
			for (auto i = 0; i < VulkanMainQueueTypeCount; i++)
			{
				aci.queueFamilyIndex = vkQueueFamilies[i];
				VkCheck(vkCreateCommandPool(vkDevice, &aci, NULL, &tl.asyncCommandPools[i]));
			}
		}
		auto pci = VkCommandPoolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = vkQueueFamilies[VulkanPresentQueue],
		};
		VkCheck(vkCreateCommandPool(vkDevice, &pci, NULL, &vkPresentCommandPool));
		auto fci = VkCommandPoolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		};
		for (auto i = 0; i < VulkanCommandGroupCount; i += 1)
		{
			for (auto j = 0; j < VulkanMainQueueTypeCount; j += 1)
			{
				vkFrameQueuedCommandBuffers[i][j].Resize(WorkerThreadCount());
				vkFrameCommandBufferPools[i][j].Resize(WorkerThreadCount());
				vkFrameCommandPools[i][j].Resize(WorkerThreadCount());
				for (auto k = 0; k < WorkerThreadCount(); k += 1)
				{
					fci.queueFamilyIndex = vkQueueFamilies[j];
					VkCheck(vkCreateCommandPool(vkDevice, &fci, NULL, &vkFrameCommandPools[i][j][k]));
					vkFrameQueuedCommandBuffers[i][j][k].SetAllocator(GlobalAllocator());
					vkFrameCommandBufferPools[i][j][k].SetAllocator(GlobalAllocator());
				}
			}
		}
	}
	// Create swapchain.
	{
		auto surfCaps = VkSurfaceCapabilitiesKHR{};
		VkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &surfCaps));
		auto ext = VkExtent2D{};
		if (surfCaps.currentExtent.width == U32Max && surfCaps.currentExtent.height == U32Max) // Indicates Vulkan will accept any extent dimension.
		{
			ext.width = RenderWidth();
			ext.height = RenderHeight();
		}
		else
		{
			ext.width = Maximum(surfCaps.minImageExtent.width, Minimum(surfCaps.maxImageExtent.width, RenderWidth()));
			ext.height = Maximum(surfCaps.minImageExtent.height, Minimum(surfCaps.maxImageExtent.height, RenderHeight()));
		}
		// @TODO: Why is this a problem?
		if (ext.width != RenderWidth() && ext.height != RenderHeight())
		{
			Abort("Vulkan", "Swapchain image dimensions do not match the window dimensions: swapchain %ux%u, window %ux%u.", ext.width, ext.height, RenderWidth(), RenderHeight());
		}
		auto minImgs = surfCaps.minImageCount + 1;
		if (surfCaps.maxImageCount > 0 && (minImgs > surfCaps.maxImageCount))
		{
			minImgs = surfCaps.maxImageCount;
		}
		auto ci = VkSwapchainCreateInfoKHR
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = vkSurface,
			.minImageCount = minImgs,
			.imageFormat = vkSurfaceFormat.format,
			.imageColorSpace  = vkSurfaceFormat.colorSpace,
			.imageExtent = ext,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform = surfCaps.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = vkPresentMode,
			.clipped = 1,
			.oldSwapchain = NULL,
		};
		if (vkQueueFamilies[VulkanPresentQueue] != vkQueueFamilies[VulkanGraphicsQueue])
		{
			auto fams = MakeStaticArray<u32>(
				vkQueueFamilies[VulkanGraphicsQueue],
				vkQueueFamilies[VulkanPresentQueue]);
			ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			ci.queueFamilyIndexCount = fams.Count();
			ci.pQueueFamilyIndices = fams.elements;
		}
		else
		{
			ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		VkCheck(vkCreateSwapchainKHR(vkDevice, &ci, NULL, &vkSwapchain));
	}
	// Get swapchain images.
	{
		auto numImgs = u32{};
		VkCheck(vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &numImgs, NULL));
		vkSwapchainImages.Resize(numImgs);
		VkCheck(vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &numImgs, &vkSwapchainImages[0]));
		vkSwapchainImageViews.Resize(numImgs);
		for (auto i = 0; i < numImgs; i++)
		{
			auto cm = VkComponentMapping 
			{
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			};
			auto isr = VkImageSubresourceRange 
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			};
			auto ci = VkImageViewCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = vkSwapchainImages[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = vkSurfaceFormat.format,
				.components = cm,
				.subresourceRange = isr,
			};
			VkCheck(vkCreateImageView(vkDevice, &ci, NULL, &vkSwapchainImageViews[i]));
		}
	}
	{
		auto ci = VkFenceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
		for (auto i = 0; i < VulkanMaxFramesInFlight; i += 1)
		{
			VkCheck(vkCreateFence(vkDevice, &ci, NULL, &vkFrameFences[i]));
		}
	}
	{
		auto ps = VkDescriptorPoolSize
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 100, // @TODO
		};
		auto ci = VkDescriptorPoolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
			.maxSets = 100, // @TODO
			.poolSizeCount = 1,
			.pPoolSizes = &ps,
		};
		VkCheck(vkCreateDescriptorPool(vkDevice, &ci, NULL, &vkDescriptorPool));
	}
	// Descriptors.
	{
		PushContextAllocator(GlobalAllocator());
		Defer(PopContextAllocator());
		vkDescriptorSets.Resize(vkSwapchainImages.count);
		vkDescriptorSetBuffers.Resize(vkSwapchainImages.count);
		for (auto i = 0; i < vkSwapchainImages.count; i += 1)
		{
			vkDescriptorSets[i].Resize(ShaderDescriptorSetCount);
			vkDescriptorSetBuffers[i].Resize(ShaderDescriptorSetCount);
		}
		auto fci = VkFenceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		};
		VkCheck(vkCreateFence(vkDevice, &fci, NULL, &vkDescriptorSetUpdateFence));
		auto MakeDescriptorSetGroup = [](s64 set, ArrayView<VkDescriptorSetLayoutBinding> bindings)
		{
			auto slci = VkDescriptorSetLayoutCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
				.bindingCount = (u32)bindings.count,
				.pBindings = bindings.elements,
			};
			VkCheck(vkCreateDescriptorSetLayout(vkDevice, &slci, NULL, &vkDescriptorSetLayouts[set]));
			auto layouts = NewArray<VkDescriptorSetLayout>(vkSwapchainImages.count);
			for (auto i = 0; i < vkSwapchainImages.count; i++)
			{
				layouts[i] = vkDescriptorSetLayouts[set];
			}
			auto ai = VkDescriptorSetAllocateInfo
			{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.descriptorPool = vkDescriptorPool,
				.descriptorSetCount = (u32)vkSwapchainImages.count,
				.pSetLayouts = layouts.elements,
			};
			auto sets = NewArray<VkDescriptorSet>(vkSwapchainImages.count);
			VkCheck(vkAllocateDescriptorSets(vkDevice, &ai, sets.elements));
			for (auto i = 0; i < vkSwapchainImages.count; i += 1)
			{
				vkDescriptorSets[i][set] = sets[i];
			}
			for (auto i = 0; i < vkSwapchainImages.count; i += 1)
			{
				vkDescriptorSetBuffers[i][set] = vkGPUUniformBlockAllocator.AllocateBuffer(VulkanInitialDescriptorSetBufferSize, NULL);
			}
		};
		auto global = MakeStaticArray<VkDescriptorSetLayoutBinding>(
			VkDescriptorSetLayoutBinding
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			});
		MakeDescriptorSetGroup(ShaderGlobalDescriptorSet, global);
		auto view = MakeStaticArray<VkDescriptorSetLayoutBinding>(
			VkDescriptorSetLayoutBinding
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			});
		MakeDescriptorSetGroup(ShaderViewDescriptorSet, view);
		auto material = MakeStaticArray<VkDescriptorSetLayoutBinding>(
			VkDescriptorSetLayoutBinding
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			});
		MakeDescriptorSetGroup(ShaderMaterialDescriptorSet, material);
		auto object = MakeStaticArray<VkDescriptorSetLayoutBinding>(
			VkDescriptorSetLayoutBinding
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			});
		MakeDescriptorSetGroup(ShaderObjectDescriptorSet, object);
	}
	// Pipeline layout.
	{
		auto ci = VkPipelineLayoutCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = (u32)vkDescriptorSetLayouts.Count(),
			.pSetLayouts = vkDescriptorSetLayouts.elements,
			.pushConstantRangeCount = 0,
		};
		VkCheck(vkCreatePipelineLayout(vkDevice, &ci, NULL, &vkPipelineLayout));
	}
	// Pipeline cache.
	{
		auto ci = VkPipelineCacheCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		};
		vkCreatePipelineCache(vkDevice, &ci, NULL, &vkPipelineCache);
	}
	// Default depth image.
	{
		auto ici = VkImageCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VulkanDepthBufferFormat,
			.extent =
			{
				.width = (u32)RenderWidth(),
				.height = (u32)RenderHeight(),
				.depth = 1,
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VulkanDepthBufferSampleCount,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VulkanDepthBufferImageUsage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VulkanDepthBufferInitialLayout,
		};
		VkCheck(vkCreateImage(vkDevice, &ici, NULL, &vkDefaultDepthImage));
		auto mr = VkMemoryRequirements{};
		vkGetImageMemoryRequirements(vkDevice, vkDefaultDepthImage, &mr);
		// @TODO: Use the image allocator.
		auto ai = VkMemoryAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = (VkDeviceSize)mr.size,
			.memoryTypeIndex = vkMemoryTypeToMemoryIndex[VulkanGPUMemory],
		};
		auto mem = VkDeviceMemory{};
		VkCheck(vkAllocateMemory(vkDevice, &ai, NULL, &mem));
		VkCheck(vkBindImageMemory(vkDevice, vkDefaultDepthImage, mem, 0));
		auto vci = VkImageViewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = vkDefaultDepthImage,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VulkanDepthBufferFormat,
			.components = 
			{
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange =
			{
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		VkCheck(vkCreateImageView(vkDevice, &vci, NULL, &vkDefaultDepthImageView));
	}
	vkDefaultFramebufferAttachments.Resize(vkSwapchainImages.count);
	for (auto i = 0; i < vkDefaultFramebufferAttachments.count; i += 1)
	{
		vkDefaultFramebufferAttachments[i].SetAllocator(GlobalAllocator());
		vkDefaultFramebufferAttachments[i].Append({vkSwapchainImageViews[i]});
		vkDefaultFramebufferAttachments[i].Append({vkDefaultDepthImageView});
	}
	{
		vkChangeImageOwnershipFromGraphicsToPresentQueueCommands.Resize(vkSwapchainImages.count);
		auto ai = VkCommandBufferAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = vkPresentCommandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = (u32)vkSwapchainImages.count,
		};
		VkCheck(vkAllocateCommandBuffers(vkDevice, &ai, vkChangeImageOwnershipFromGraphicsToPresentQueueCommands.elements));
		for (auto i = 0; i < vkSwapchainImages.count; i += 1)
		{
			auto bi = VkCommandBufferBeginInfo
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
			};
			VkCheck(vkBeginCommandBuffer(vkChangeImageOwnershipFromGraphicsToPresentQueueCommands[i], &bi));
			auto mb = VkImageMemoryBarrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.srcQueueFamilyIndex = vkQueueFamilies[VulkanGraphicsQueue],
				.dstQueueFamilyIndex = vkQueueFamilies[VulkanPresentQueue],
				.image = vkSwapchainImages[i],
				.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
			};
			vkCmdPipelineBarrier(vkChangeImageOwnershipFromGraphicsToPresentQueueCommands[i], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 1, &mb);
			VkCheck(vkEndCommandBuffer(vkChangeImageOwnershipFromGraphicsToPresentQueueCommands[i]));
		}
	}
}

const auto VulkanVertexBufferBindID = 0;

VkPipeline MakeVulkanPipeline(GPUShaderID id, VulkanShader s, VkRenderPass rp)
{
	switch (id)
	{
	case GPUModelShaderID:
	{
		auto assemblyCI = VkPipelineInputAssemblyStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};
		auto viewport = VkViewport
		{
			.x = 0.0f,
			.y = 0.0f,
			.width = (f32)RenderWidth(),
			.height = (f32)RenderHeight(),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		auto scissor = VkRect2D
		{
			.offset = {0, 0},
			.extent = (VkExtent2D){(u32)RenderWidth(), (u32)RenderHeight()},
		};
		auto viewportCI = VkPipelineViewportStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = &viewport,
			.scissorCount = 1,
			.pScissors = &scissor,
		};
		auto rasterCI = VkPipelineRasterizationStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = false,
			.lineWidth = 1.0f,
		};
		auto multisampleCI = VkPipelineMultisampleStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
		};
		auto depthCI = VkPipelineDepthStencilStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
		};
		auto blendStates = MakeStaticArray<VkPipelineColorBlendAttachmentState>(
			VkPipelineColorBlendAttachmentState
			{
				.blendEnable = true,
				.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
				.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				.colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
				.alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}
		);
		auto blendCI = VkPipelineColorBlendStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = (u32)blendStates.Count(),
			.pAttachments = blendStates.elements,
			.blendConstants = {},
		};
		auto vertAttrs = MakeStaticArray<VkVertexInputAttributeDescription>(
			VkVertexInputAttributeDescription
			{
				.location = 0,
				.binding = VulkanVertexBufferBindID,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex1P1N, position),
			},
			VkVertexInputAttributeDescription
			{
				.location = 1,
				.binding = VulkanVertexBufferBindID,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex1P1N, normal),
			}
		);
		auto vertBindings = MakeStaticArray<VkVertexInputBindingDescription>(
			VkVertexInputBindingDescription
			{
				.binding = VulkanVertexBufferBindID,
				.stride = sizeof(Vertex1P1N),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
			}
		);
		auto vertexCI = VkPipelineVertexInputStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = (u32)vertBindings.Count(),
			.pVertexBindingDescriptions = vertBindings.elements,
			.vertexAttributeDescriptionCount = (u32)vertAttrs.Count(),
			.pVertexAttributeDescriptions = vertAttrs.elements,
		};
		auto dynStates = MakeStaticArray<VkDynamicState>(
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		);
		auto dynStateCI = VkPipelineDynamicStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = (u32)dynStates.Count(),
			.pDynamicStates = dynStates.elements,
		};
		auto stages = Array<VkPipelineShaderStageCreateInfo>{};
		for (auto i = 0; i < s.vkModules.count; i += 1)
		{
			stages.Append(
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.stage = s.vkStages[i],
					.module = s.vkModules[i],
					.pName = "main",
				}
			);
		}
		auto ci = VkGraphicsPipelineCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = (u32)stages.count,
			.pStages = stages.elements,
			.pVertexInputState = &vertexCI,
			.pInputAssemblyState = &assemblyCI,
			.pViewportState = &viewportCI,
			.pRasterizationState = &rasterCI,
			.pMultisampleState = &multisampleCI,
			.pDepthStencilState = &depthCI,
			.pColorBlendState = &blendCI,
			.pDynamicState = &dynStateCI,
			.layout = vkPipelineLayout,
			.renderPass = rp,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1,
		};
		auto p = VkPipeline{};
		VkCheck(vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &ci, NULL, &p));
		return p;
	} break;
	case GPUShaderIDCount:
	default:
	{
		LogError("Vulkan", "Failed to find pipeline creation code for shader %k.", GPUShaderName(id));
	} break;
	}
	return {};
}

VkCommandBuffer NewVulkanCommandBuffer(Array<VkCommandBuffer> *recycle, VkCommandPool p)
{
	auto bi = VkCommandBufferBeginInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	auto cb = VkCommandBuffer{};
	if (recycle->count > 0)
	{
		cb = recycle->Pop();
		vkBeginCommandBuffer(cb, &bi);
		return cb;
	}
    auto ai = VkCommandBufferAllocateInfo
    {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = p,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
    };
    vkAllocateCommandBuffers(vkDevice, &ai, &cb);
	vkBeginCommandBuffer(cb, &bi);
	return cb;
}

VkCommandBuffer NewVulkanFrameCommandBuffer(VulkanQueueType t)
{
	return NewVulkanCommandBuffer(&vkFrameCommandBufferPools[vkCommandGroupUseIndex][t][ThreadIndex()], vkFrameCommandPools[vkCommandGroupUseIndex][t][ThreadIndex()]);
}

GPUFrameGraphicsCommandBuffer NewGPUFrameGraphicsCommandBuffer()
{
	auto cb = GPUFrameGraphicsCommandBuffer{};
    cb.vkCommandBuffer = NewVulkanFrameCommandBuffer(VulkanGraphicsQueue);
	return cb;
}

void GPUFrameGraphicsCommandBuffer::Queue()
{
	VkCheck(vkEndCommandBuffer(this->vkCommandBuffer));
	vkFrameQueuedCommandBuffers[vkCommandGroupUseIndex][VulkanGraphicsQueue][ThreadIndex()].Append(this->vkCommandBuffer);
}

GPUFrameTransferCommandBuffer NewGPUFrameTransferCommandBuffer()
{
	auto cb = GPUFrameTransferCommandBuffer{};
    cb.vkCommandBuffer = NewVulkanFrameCommandBuffer(VulkanTransferQueue);
	return cb;
}

void GPUFrameTransferCommandBuffer::Queue()
{
	VkCheck(vkEndCommandBuffer(this->vkCommandBuffer));
	vkFrameQueuedCommandBuffers[vkCommandGroupUseIndex][VulkanTransferQueue][ThreadIndex()].Append(this->vkCommandBuffer);
}

GPUFrameComputeCommandBuffer NewGPUFrameComputeCommandBuffer()
{
	auto cb = GPUFrameComputeCommandBuffer{};
    cb.vkCommandBuffer = NewVulkanFrameCommandBuffer(VulkanComputeQueue);
	return cb;
}

void GPUFrameComputeCommandBuffer::Queue()
{
	VkCheck(vkEndCommandBuffer(this->vkCommandBuffer));
	vkFrameQueuedCommandBuffers[vkCommandGroupUseIndex][VulkanComputeQueue][ThreadIndex()].Append(this->vkCommandBuffer);
}

VkCommandBuffer NewVulkanAsyncCommandBuffer(VulkanQueueType t)
{
	vkThreadLocal[ThreadIndex()].asyncCommandBufferLock.Lock();
	Defer(vkThreadLocal[ThreadIndex()].asyncCommandBufferLock.Unlock());
	return NewVulkanCommandBuffer({} /* @TODO */, vkThreadLocal[ThreadIndex()].asyncCommandPools[t]);
}

void QueueVulkanAsyncCommandBuffer(VkCommandBuffer cb, VulkanQueueType t, bool *signalOnCompletion)
{
	VkCheck(vkEndCommandBuffer(cb));
	vkThreadLocal[ThreadIndex()].asyncCommandBufferLock.Lock();
	Defer(vkThreadLocal[ThreadIndex()].asyncCommandBufferLock.Unlock());
	vkThreadLocal[ThreadIndex()].asyncQueuedCommandBuffers[t].Append(cb);
	vkThreadLocal[ThreadIndex()].asyncSignals[t].Append(signalOnCompletion);
}

GPUAsyncGraphicsCommandBuffer NewGPUAsyncGraphicsCommandBuffer()
{
	auto cb = GPUAsyncGraphicsCommandBuffer{};
	cb.vkCommandBuffer = NewVulkanAsyncCommandBuffer(VulkanGraphicsQueue);
	return cb;
}

void GPUAsyncGraphicsCommandBuffer::Queue(bool *signalOnCompletion)
{
	QueueVulkanAsyncCommandBuffer(this->vkCommandBuffer, VulkanGraphicsQueue, signalOnCompletion);
}

GPUAsyncTransferCommandBuffer NewGPUAsyncTransferCommandBuffer()
{
	auto cb = GPUAsyncTransferCommandBuffer{};
	cb.vkCommandBuffer = NewVulkanAsyncCommandBuffer(VulkanTransferQueue);
	return cb;
}

void GPUAsyncTransferCommandBuffer::Queue(bool *signalOnCompletion)
{
	QueueVulkanAsyncCommandBuffer(this->vkCommandBuffer, VulkanTransferQueue, signalOnCompletion);
}

GPUAsyncComputeCommandBuffer NewGPUAsyncComputeCommandBuffer()
{
	auto cb = GPUAsyncComputeCommandBuffer{};
	cb.vkCommandBuffer = NewVulkanAsyncCommandBuffer(VulkanComputeQueue);
	return cb;
}

void GPUAsyncComputeCommandBuffer::Queue(bool *signalOnCompletion)
{
	QueueVulkanAsyncCommandBuffer(this->vkCommandBuffer, VulkanComputeQueue, signalOnCompletion);
}

VkFramebuffer MakeVulkanFramebuffer(VkRenderPass rp, GPUFramebuffer fb)
{
	auto vkFB = vkFramebufferCache.Lookup({rp, fb.id}, 0);
	if (vkFB)
	{
		return vkFB;
	}
	auto ci = VkFramebufferCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = rp,
		.attachmentCount = (u32)fb.attachments.count,
		.pAttachments = (VkImageView *)fb.attachments.elements, // @TODO: ... not thrilled about this
		.width = (u32)fb.w,
		.height = (u32)fb.h,
		.layers = 1,
	};
	VkCheck(vkCreateFramebuffer(vkDevice, &ci, NULL, &vkFB));
	vkFramebufferCache.Insert({rp, fb.id}, vkFB);
	return vkFB;
}

void GPUCommandBuffer::BeginRender(GPUShaderID sid, GPUFramebuffer fb)
{
	Assert(fb.id > vkSwapchainImages.count || fb.id == vkSwapchainImageIndex);
	auto rp = vkRenderPasses[sid];
	if (!rp)
	{
		LogError("Vulkan", "Failed command buffer BeginRendering: could not find shader renderpass.");
		return;
	}
	auto vkFB = MakeVulkanFramebuffer(rp, fb);
	auto p = vkPipelines[sid];
	if (!p)
	{
		LogError("Vulkan", "Failed command buffer BeginRendering: could not find shader pipeline.");
		return;
	}
	vkCmdBindPipeline(this->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p);
	auto clearColor = VkClearValue
	{
		.color.float32 = {0.04f, 0.19f, 0.34f, 1.0f},
	};
	auto clearDepthStencil = VkClearValue
	{
		.depthStencil.depth = 1.0f,
		.depthStencil.stencil = 0,
	};
	auto clearValues = MakeStaticArray<VkClearValue>(clearColor, clearDepthStencil);
	auto bi = VkRenderPassBeginInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = rp,
		.framebuffer = vkFB,
		.renderArea =
		{
			.offset = {0, 0},
			.extent = {(u32)RenderWidth(), (u32)RenderHeight()},
		},
		.clearValueCount = (u32)clearValues.Count(),
		.pClearValues = clearValues.elements,
	};
	vkCmdBindDescriptorSets(this->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, ShaderDescriptorSetCount, vkDescriptorSets[vkSwapchainImageIndex].elements, 0, NULL);
	vkCmdBeginRenderPass(this->vkCommandBuffer, &bi, VK_SUBPASS_CONTENTS_INLINE);
}

void GPUCommandBuffer::EndRender()
{
	vkCmdEndRenderPass(this->vkCommandBuffer);
}

void GPUCommandBuffer::SetViewport(s64 w, s64 h)
{
	auto v = VkViewport
	{
		.x = 0.0f,
		.y = 0.0f,
		.width = (f32)w,
		.height = (f32)h,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(this->vkCommandBuffer, 0, 1, &v);
}

void GPUCommandBuffer::SetScissor(s64 w, s64 h)
{
	auto scissor = VkRect2D
	{
		.extent = {(u32)w, (u32)h},
	};
	vkCmdSetScissor(this->vkCommandBuffer, 0, 1, &scissor);
}

void GPUCommandBuffer::BindVertexBuffer(GPUBuffer b, s64 bindPoint)
{
	auto offset = (VkDeviceSize)b.offset;
	vkCmdBindVertexBuffers(this->vkCommandBuffer, bindPoint, 1, &b.vkBuffer, &offset);
}

void GPUCommandBuffer::BindIndexBuffer(GPUBuffer b, GPUIndexType t)
{
	vkCmdBindIndexBuffer(this->vkCommandBuffer, b.vkBuffer, b.offset, t);
}

void GPUCommandBuffer::DrawIndexed(s64 numIndices, s64 firstIndex, s64 vertexOffset)
{
	vkCmdDrawIndexed(this->vkCommandBuffer, numIndices, 1, firstIndex, vertexOffset, 0);
}

void GPUCommandBuffer::DrawIndexedIndirect(GPUBuffer cmdBuf, s64 count)
{
	vkCmdDrawIndexedIndirect(this->vkCommandBuffer, cmdBuf.vkBuffer, cmdBuf.offset, count, sizeof(VkDrawIndexedIndirectCommand));
}

void GPUCommandBuffer::CopyBuffer(s64 size, GPUBuffer src, GPUBuffer dst, s64 srcOffset, s64 dstOffset)
{
	auto c = VkBufferCopy
	{
		.srcOffset = (VkDeviceSize)src.offset + srcOffset,
		.dstOffset = (VkDeviceSize)dst.offset + dstOffset,
		.size = (VkDeviceSize)size,
	};
	vkCmdCopyBuffer(this->vkCommandBuffer, src.vkBuffer, dst.vkBuffer, 1, &c);
}

#if 0
void GPUCommandBuffer::CopyBufferToImage(GPUBuffer b, GPUImage i, u32 w, u32 h)
{
	auto region = VkBufferImageCopy
	{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = {},
		.imageExtent =
		{
			w,
			h,
			1,
		},
	};
	vkCmdCopyBufferToImage(this->vkCommandBuffer, b.vkBuffer, i.vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}
#endif

void SubmitVulkanFrameCommandBuffers(VulkanQueueType t, ArrayView<VkSemaphore> waitSems, ArrayView<VkPipelineStageFlags> waitStages, ArrayView<VkSemaphore> signalSems, VkFence f)
{
	auto cbs = Array<VkCommandBuffer>{};
	for (auto q : vkFrameQueuedCommandBuffers[vkCommandGroupUseIndex][t])
	{
		cbs.AppendAll(q);
	}
	auto si = VkSubmitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = (u32)waitSems.count,
		.pWaitSemaphores = waitSems.elements,
		.pWaitDstStageMask = waitStages.elements,
		.commandBufferCount = (u32)cbs.count,
		.pCommandBuffers = cbs.elements,
		.signalSemaphoreCount = (u32)signalSems.count,
		.pSignalSemaphores = signalSems.elements,
	};
	VkCheck(vkQueueSubmit(vkQueues[t], 1, &si, f));
}

GPUFence GPUSubmitFrameGraphicsCommandBuffers()
{
	#if 0
		// @TODO: Do this stuff?
	    if (demo->separate_present_queue) {
        // We have to transfer ownership from the graphics queue family to the
        // present queue family to be able to present.  Note that we don't have
        // to transfer from present queue family back to graphics queue family at
        // the start of the next frame because we don't care about the image's
        // contents at that point.
        VkImageMemoryBarrier image_ownership_barrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                                        .pNext = NULL,
                                                        .srcAccessMask = 0,
                                                        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                        .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                        .srcQueueFamilyIndex = demo->graphics_queue_family_index,
                                                        .dstQueueFamilyIndex = demo->present_queue_family_index,
                                                        .image = demo->swapchain_image_resources[demo->current_buffer].image,
                                                        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

        vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                             NULL, 0, NULL, 1, &image_ownership_barrier);
	#endif
	auto waitSems = MakeStaticArray<VkSemaphore>(
		vkFrameTransfersCompleteSemaphores[vkFrameIndex],
		vkImageAcquiredSemaphores[vkFrameIndex]);
	auto waitStages = MakeStaticArray<VkPipelineStageFlags>(
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	auto signalSems = MakeStaticArray<VkSemaphore>(vkFrameGraphicsCompleteSemaphores[vkFrameIndex]);
	SubmitVulkanFrameCommandBuffers(VulkanGraphicsQueue, waitSems, waitStages, signalSems, vkFrameFences[vkFrameIndex]);
	return GPUFence{vkFrameFences[vkFrameIndex]};
}

GPUFence GPUSubmitFrameTransferCommandBuffers()
{
	auto signalSems = MakeStaticArray<VkSemaphore>(vkFrameTransfersCompleteSemaphores[vkFrameIndex]);
	SubmitVulkanFrameCommandBuffers(VulkanTransferQueue, {}, {}, signalSems, VK_NULL_HANDLE);
	return {}; // @TODO
}

GPUFence GPUSubmitFrameComputeCommandBuffers()
{
	//return SubmitVulkanFrameCommandBuffers(VulkanTransferQueue, waitSems, waitStages, signalSems);
	return {};
}

s64 GPUSwapchainImageCount()
{
	return vkSwapchainImages.count;
}

VkBuffer NewVulkanBuffer(VkBufferUsageFlags u, s64 size)
{
	auto b = VkBuffer{};
	auto ci = VkBufferCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = (VkDeviceSize)size,
		.usage = u,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VkCheck(vkCreateBuffer(vkDevice, &ci, NULL, &b));
	return b;
}

GPUBuffer NewGPUVertexBuffer(s64 size)
{
	return vkGPUVertexBlockAllocator.AllocateBuffer(size, NULL);
}

GPUBuffer NewGPUIndexBuffer(s64 size)
{
	return vkGPUIndexBlockAllocator.AllocateBuffer(size, NULL);
}

GPUBuffer NewGPUFrameIndirectBuffer(s64 size)
{
	return vkGPUIndirectFrameAllocator.AllocateBuffer(size, NULL);
}

GPUFrameStagingBuffer NewGPUFrameStagingBufferX(s64 size, GPUBuffer dst)
{
	auto sb = GPUFrameStagingBuffer
	{
		.size = size,
		.destination = dst,
	};
	sb.source = vkCPUStagingFrameAllocator.AllocateBuffer(size, &sb.map);
	return sb;
}

void GPUFrameStagingBuffer::Flush()
{
	auto cb = NewGPUFrameTransferCommandBuffer();
	cb.CopyBuffer(this->size, this->source, this->destination, 0, 0);
	cb.Queue();
}

void GPUFrameStagingBuffer::FlushIn(GPUCommandBuffer cb)
{
	cb.CopyBuffer(this->size, this->source, this->destination, 0, 0);
}

void *GPUFrameStagingBuffer::Map()
{
	return this->map;
}

#if 0
GPUAsyncStagingBuffer NewGPUAsyncStagingBuffer(s64 size, GPUBuffer dst)
{
	auto sb = GPUAsyncStagingBuffer
	{
		.vkBuffer = NewVulkanBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size),
		.vkDestinationBuffer = dst.vkBuffer,
	};
	auto mr = VkMemoryRequirements{};
	vkGetBufferMemoryRequirements(vkDevice, sb.vkBuffer, &mr);
	// @TODO sb.memory = vkCPUStagingBlockAllocator.Allocate(mr.size, mr.alignment);
	VkCheck(vkBindBufferMemory(vkDevice, sb.vkBuffer, sb.memory->vkMemory, sb.memory->offset));
	return sb;
}
#endif

void GPUAsyncStagingBuffer::Flush()
{
	auto cb = NewGPUAsyncTransferCommandBuffer();
	//cb.CopyBuffer(this->memory->size, this->source, this->destination, 0, 0);
	auto tl = &vkThreadLocal[ThreadIndex()];
	tl->asyncStagingBufferLock.Lock();
	Defer(tl->asyncStagingBufferLock.Unlock());
	tl->asyncStagingSignals.Resize(tl->asyncStagingSignals.count + 1);
	cb.Queue(tl->asyncStagingSignals.Last());
	//tl->asyncPendingStagingBuffers.Append({this->memory, this->vkBuffer});
}

void GPUAsyncStagingBuffer::FlushIn(GPUCommandBuffer cb)
{
	//auto src = GPUBuffer
	//{
		//.vkBuffer = this->vkBuffer,
	//};
	//auto dst = GPUBuffer
	//{
		//.vkBuffer = this->vkDestinationBuffer,
	//};
	//cb.CopyBuffer(this->memory->size, src, dst, 0, 0);
}

void *GPUAsyncStagingBuffer::Map()
{
	//return this->memory->map;
	return NULL;
}

GPUImage NewGPUImage(s64 w, s64 h, GPUFormat f, GPUImageLayout il, GPUImageUsageFlags uf, GPUSampleCount sc)
{
	auto ci = VkImageCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = f,
		.extent =
		{
			.width = (u32)w,
			.height = (u32)h,
			.depth = 1,
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = sc,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = uf,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = il,
	};
	auto i = GPUImage{};
	VkCheck(vkCreateImage(vkDevice, &ci, NULL, &i.vkImage));
	auto mr = VkMemoryRequirements{};
	vkGetImageMemoryRequirements(vkDevice, i.vkImage, &mr);
	// @TODO auto mem = vkGPUBlockAllocator.Allocate(mr.size, mr.alignment);
	// @TODO: VkCheck(vkBindImageMemory(vkDevice, i.vkImage, mem->vkMemory, mem->offset));
	//i.memory = mem;
	return i;
}

GPUImageView NewGPUImageView(GPUImage src, GPUImageViewType t, GPUFormat f, GPUSwizzleMapping sm, GPUImageSubresourceRange isr)
{
	auto ci = VkImageViewCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = src.vkImage,
		.viewType = t,
		.format = f,
		.components = 
		{
			.r = sm.r,
			.g = sm.g,
			.b = sm.b,
			.a = sm.a,
		},
		.subresourceRange =
		{
			.aspectMask = isr.aspectMask,
			.baseMipLevel = isr.baseMipLevel,
			.levelCount = isr.levelCount,
			.baseArrayLayer = isr.baseArrayLayer,
			.layerCount = isr.layerCount,
		},
	};
	auto iv = GPUImageView{};
	VkCheck(vkCreateImageView(vkDevice, &ci, NULL, &iv.vkImageView));
	return iv;
}

GPUFence NewGPUFence()
{
	auto f = GPUFence{};
	auto ci = VkFenceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	};
	VkCheck(vkCreateFence(vkDevice, &ci, NULL, &f.vkFence));
	return f;
}

bool GPUFence::Check()
{
	auto s = vkGetFenceStatus(vkDevice, this->vkFence);
	if (s == VK_SUCCESS)
	{
		return true;
	}
	if (s == VK_NOT_READY)
	{
		return false;
	}
	VkCheck(s);
	return false;
}

bool GPUFence::Wait(u64 timeout)
{
	auto s = vkWaitForFences(vkDevice, 1, &this->vkFence, true, timeout);
	if (s == VK_SUCCESS)
	{
		return true;
	}
	if (s == VK_TIMEOUT)
	{
		return false;
	}
	VkCheck(s);
	return false;
}

void GPUFence::Free()
{
	// @TODO
}

bool WaitForFences(ArrayView<GPUFence> fs, bool waitAll, u64 timeout)
{
	auto s = vkWaitForFences(vkDevice, fs.count, (VkFence *)fs.elements, waitAll, timeout);
	if (s == VK_SUCCESS)
	{
		return true;
	}
	if (s == VK_TIMEOUT)
	{
		return false;
	}
	VkCheck(s);
	return false;
}

void FreeGPUFences(ArrayView<GPUFence> fs)
{
	VkCheck(vkResetFences(vkDevice, fs.count, (VkFence *)fs.elements));
	for (auto f : fs)
	{
		// @TODO
	}
}

const auto VulkanAcquireImageTimeout = SecondsToNanoseconds(2);

void GPUBeginFrame()
{
	VkCheck(vkWaitForFences(vkDevice, 1, &vkFrameFences[vkFrameIndex], true, U32Max));
	VkCheck(vkResetFences(vkDevice, 1, &vkFrameFences[vkFrameIndex]));
	VkCheck(vkAcquireNextImageKHR(vkDevice, vkSwapchain, VulkanAcquireImageTimeout, vkImageAcquiredSemaphores[vkFrameIndex], VK_NULL_HANDLE, &vkSwapchainImageIndex));
	for (auto &tl : vkFrameCommandPools[vkCommandGroupFreeIndex])
	{
		for (auto i = 0; i < WorkerThreadCount(); i += 1)
		{
			VkCheck(vkResetCommandPool(vkDevice, tl[i], 0));
		}
	}
	for (auto i = 0; i < VulkanMainQueueTypeCount; i += 1)
	{
		for (auto j = 0; j < WorkerThreadCount(); j += 1)
		{
			vkFrameCommandBufferPools[vkCommandGroupFreeIndex][i][j].AppendAll(vkFrameQueuedCommandBuffers[vkCommandGroupFreeIndex][i][j]);
		}
	}
	for (auto i = 0; i < VulkanMainQueueTypeCount; i += 1)
	{
		for (auto j = 0; j < WorkerThreadCount(); j += 1)
		{
			vkFrameQueuedCommandBuffers[vkCommandGroupFreeIndex][i][j].Resize(0);
		}
	}
	for (auto &tl : vkThreadLocal)
	{
		tl.asyncCommandBufferLock.Lock();
		for (auto i = 0; i < VulkanMainQueueTypeCount; i += 1)
		{
			for (auto j = 0; j < tl.asyncFences[i].count; )
			{
				if (!tl.asyncFences[i][j].Check())
				{
					j += 1;
					continue;
				}
				tl.asyncFences[i][j].Free();
				tl.asyncFences[i].UnorderedRemove(j);
				*tl.asyncSignals[i][j] = true;
				tl.asyncSignals[i].UnorderedRemove(j);
				// @TODO: tl.asyncPendingCommandBuffers[i][j].Free();
				tl.asyncPendingCommandBuffers[i][j].Resize(0);
			}
		}
		tl.asyncCommandBufferLock.Unlock();
		tl.asyncStagingBufferLock.Lock();
		for (auto i = 0; i < tl.asyncStagingSignals.count; )
		{
			if (!tl.asyncStagingSignals[i])
			{
				i += 1;
				continue;
			}
			// @TODO: Take locks...
			tl.asyncStagingSignals.UnorderedRemove(i);
			// @TODO: tl.asyncPendingStagingBuffers[i].Free();
			tl.asyncPendingStagingBuffers.UnorderedRemove(i);
		}
		tl.asyncStagingBufferLock.Unlock();
	}
	vkCPUStagingFrameAllocator.FreeOldestFrame();
	vkGPUIndirectFrameAllocator.FreeOldestFrame();
}

void GPUEndFrame()
{
	if (vkQueueFamilies[VulkanPresentQueue] != vkQueueFamilies[VulkanGraphicsQueue])
	{
		// If we are using separate queues, change image ownership to the present queue before
		// presenting, waiting for the draw complete semaphore and signalling the ownership released
		// semaphore when finished.
		auto stage = VkPipelineStageFlags{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		auto si = VkSubmitInfo
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &vkFrameGraphicsCompleteSemaphores[vkFrameIndex],
			.pWaitDstStageMask = &stage,
			.commandBufferCount = 1,
			.pCommandBuffers = &vkChangeImageOwnershipFromGraphicsToPresentQueueCommands[vkSwapchainImageIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &vkImageOwnershipSemaphores[vkFrameIndex],
		};
		VkCheck(vkQueueSubmit(vkQueues[VulkanPresentQueue], 1, &si, VK_NULL_HANDLE));
	}
	// If we are using separate queues we have to wait for image ownership, otherwise wait for draw
	// complete.
	auto pi = VkPresentInfoKHR
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.swapchainCount = 1,
		.pSwapchains = &vkSwapchain,
		.pImageIndices = &vkSwapchainImageIndex,
	};
	if (vkQueueFamilies[VulkanPresentQueue] != vkQueueFamilies[VulkanGraphicsQueue])
	{
		pi.pWaitSemaphores = &vkImageOwnershipSemaphores[vkFrameIndex];
	}
	else
	{
		pi.pWaitSemaphores = &vkFrameGraphicsCompleteSemaphores[vkFrameIndex];
	}
	VkCheck(vkQueuePresentKHR(vkQueues[VulkanPresentQueue], &pi));
	vkCommandGroupUseIndex = (vkCommandGroupUseIndex + 1) % VulkanCommandGroupCount;
	vkCommandGroupFreeIndex = (vkCommandGroupUseIndex + 1) % VulkanCommandGroupCount;
	vkFrameMemoryUseIndex = vkCommandGroupUseIndex;
	vkFrameMemoryFreeIndex = vkCommandGroupFreeIndex;
	vkFrameIndex = (vkFrameIndex + 1) % VulkanMaxFramesInFlight;
}

VkRenderPass MakeVulkanRenderPass(GPUShaderID sid)
{
	switch (sid)
	{
	case GPUModelShaderID:
	{
		VkAttachmentDescription attachments[] =
		{
			{
				.format = vkSurfaceFormat.format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			},
			{
				.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			},
		};
		VkAttachmentReference colorAttachments[] =
		{
			{
				.attachment = 0,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
		};
		VkAttachmentReference stencilAttachment =
		{
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
		#define CArrayCount(x) (sizeof(x) / sizeof(x[0]))
		VkSubpassDescription subpassDescriptions[] =
		{
			{
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = CArrayCount(colorAttachments),
				.pColorAttachments = colorAttachments,
				.pDepthStencilAttachment = &stencilAttachment,
			},
		};
		VkSubpassDependency subpassDependencies[] =
		{
			{
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			},
		};
		VkRenderPassCreateInfo renderPassCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = CArrayCount(attachments),
			.pAttachments = attachments,
			.subpassCount = CArrayCount(subpassDescriptions),
			.pSubpasses = subpassDescriptions,
			.dependencyCount = CArrayCount(subpassDependencies),
			.pDependencies = subpassDependencies,
		};
		VkRenderPass renderPass;
		VkCheck(vkCreateRenderPass(vkDevice, &renderPassCreateInfo, NULL, &renderPass));
		return renderPass;
	} break;
	case GPUShaderIDCount:
	default:
	{
		LogError("Vulkan", "Unknown shader id %d.", sid);
	} break;
	}
	return {};
}

void GPUCompileShaderFromFile(GPUShaderID id, String path, bool *err)
{
	auto spirv = GenerateVulkanSPIRV(path, err);
	if (*err)
	{
		LogError("Vulkan", "Failed to generate SPIRV for file %k.", path);
		return;
	}
	auto s = VulkanShader
	{
		.vkStages = spirv.stages,
	};
	for (auto fp : spirv.filepaths)
	{
		auto spirv = ReadEntireFile(fp, err);
		if (*err)
		{
			LogError("Vulkan", "Failed to read SPIRV file %k from shader %k.", fp, path);
			return;
		}
		auto ci = VkShaderModuleCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = (u32)spirv.count,
			.pCode = (u32 *)spirv.elements,
		};
		auto m = VkShaderModule{};
		VkCheck(vkCreateShaderModule(vkDevice, &ci, NULL, &m));
		s.vkModules.Append(m);
	}
	vkShaders[id] = s;
	auto rp = MakeVulkanRenderPass(id);
	vkRenderPasses[id] = rp;
	vkPipelines[id] = MakeVulkanPipeline(id, s, rp);
}

GPUUniform GPUAddUniform(s64 set)
{
	// @TODO
	return
	{
		.set = set,
		.index = 0,
	};
}

void GPUUpdateUniforms(ArrayView<GPUUniformBufferWriteDescription> bs, ArrayView<GPUUniformImageWriteDescription> is)
{
	// @TODO: Combine writes/copies to adjacent indices?
	auto cb = NewGPUFrameTransferCommandBuffer();
	for (auto b : bs)
	{
		auto sb = NewGPUFrameStagingBufferX(b.size, vkDescriptorSetBuffers[vkSwapchainImageIndex][b.uniform.set]); // @TODO
		CopyArray(NewArrayView((u8 *)b.data, b.size), NewArrayView((u8 *)sb.Map(), b.size));
		sb.FlushIn(cb);
	}
	cb.Queue();
	auto vkBufferInfos = NewArrayWithCapacity<VkDescriptorBufferInfo>(bs.count);
	for (auto b : bs)
	{
		vkBufferInfos.Append(
			VkDescriptorBufferInfo
			{
				.buffer = vkDescriptorSetBuffers[vkSwapchainImageIndex][b.uniform.set].vkBuffer,
				.offset = (VkDeviceSize)(vkDescriptorSetBuffers[vkSwapchainImageIndex][b.uniform.set].offset + (b.size * b.uniform.index)),
				.range = (VkDeviceSize)b.size,
			});
	}
	auto vkImageInfos = NewArrayWithCapacity<VkDescriptorImageInfo>(is.count);
	for (auto i : is)
	{
		vkImageInfos.Append(
			VkDescriptorImageInfo
			{
				.sampler = i.sampler.vkSampler,
				.imageView = i.imageView.vkImageView,
				.imageLayout = i.layout,
			});
	}
	auto writes = NewArrayWithCapacity<VkWriteDescriptorSet>(bs.count + is.count);
	for (auto i = 0; i < bs.count; i+= 1)
	{
		writes.Append(
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = vkDescriptorSets[vkSwapchainImageIndex][bs[i].uniform.set],
				.dstBinding = 0,
				.dstArrayElement = (u32)bs[i].uniform.index,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pBufferInfo = &vkBufferInfos[i],
			});
	}
	for (auto i = 0; i < is.count; i+= 1)
	{
		writes.Append(
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = vkDescriptorSets[vkSwapchainImageIndex][is[i].uniform.set],
				.dstBinding = 0,
				.dstArrayElement = (u32)is[i].uniform.index,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &vkImageInfos[i],
			});
	}
	// @TODO: Handle copies.
	vkUpdateDescriptorSets(vkDevice, (u32)writes.count, writes.elements, 0, NULL); // No return.
}

GPUFramebuffer NewGPUFramebuffer(s64 w, s64 h, ArrayView<GPUImageView> attachments)
{
	static auto idGenerator = (u64)vkSwapchainImages.count;
	auto id = (u64)AtomicFetchAndAdd64((s64 *)&idGenerator, 1);
	if (id < vkSwapchainImages.count)
	{
		id = AtomicFetchAndAdd64((s64 *)&idGenerator, vkSwapchainImages.count);
	}
	return
	{
		.id = id,
		.w = w,
		.h = h,
		.attachments = attachments.CopyIn(GlobalAllocator()),
	};
}

GPUFramebuffer GPUDefaultFramebuffer()
{
	return
	{
		.id = vkSwapchainImageIndex,
		.w = RenderWidth(),
		.h = RenderHeight(),
		.attachments = vkDefaultFramebufferAttachments[vkSwapchainImageIndex],
	};
}

#endif
