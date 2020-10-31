#ifdef VulkanBuild

#include "Swapchain.h"

namespace GPU
{

Framebuffer NewFramebuffer(u32 w, u32 h, ArrayView<ImageView> attachments)
{
	static auto idGenerator = u64{0};
	auto id = (u64)AtomicFetchAndAdd64((s64 *)&idGenerator, 1);
	// The first few identifiers are reserved for the default swapchain framebuffer.
	if (id < swapchain.images.count)
	{
		id = (u64)AtomicFetchAndAdd64((s64 *)&idGenerator, swapchain.images.count);
	}
	auto fb = Framebuffer
	{
		.id = id,
		.width = w,
		.height = h,
		.attachments = NewArrayWithCapacityIn<VkImageView>(GlobalAllocator(), attachments.count),
	};
	for (auto a : attachments)
	{
		fb.attachments.Append(a.vkImageView);
	}
	return fb;
}

Framebuffer DefaultFramebuffer()
{
	return
	{
		.id = 0,
	};
}

struct FramebufferKey
{
	VkRenderPass vkRenderPass;
	u64 framebufferID;

	bool operator==(FramebufferKey k);
};

bool FramebufferKey::operator==(FramebufferKey k)
{
	return this->vkRenderPass == k.vkRenderPass && this->framebufferID == k.framebufferID;
}

u64 HashFramebufferKey(FramebufferKey k)
{
	return HashPointer(k.vkRenderPass) ^ Hash64(k.framebufferID);
}

auto framebufferCache = NewHashTable<FramebufferKey, VkFramebuffer>(0, HashFramebufferKey);

VkFramebuffer NewVkFramebuffer(VkRenderPass rp, Framebuffer fb)
{
	if (fb.id == 0)
	{
		// Default framebuffer.
		fb = Framebuffer
		{
			.id = swapchain.imageIndex,
			.width = (u32)RenderWidth(),
			.height = (u32)RenderHeight(),
			.attachments = MakeArray<VkImageView>(swapchain.imageViews[swapchain.imageIndex], swapchain.defaultDepthImageView), // @TODO: Make in FrameAllocator.
		};
	}
	auto key = FramebufferKey
	{
		.vkRenderPass = rp,
		.framebufferID = fb.id,
	};
	if (auto vkFB = framebufferCache.Lookup(key); vkFB)
	{
		return *vkFB;
	}
	auto ci = VkFramebufferCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = rp,
		.attachmentCount = (u32)fb.attachments.count,
		.pAttachments = fb.attachments.elements,
		.width = fb.width,
		.height = fb.height,
		.layers = 1,
	};
	auto vkFB = VkFramebuffer{};
	VkCheck(vkCreateFramebuffer(vkDevice, &ci, NULL, &vkFB));
	framebufferCache.Insert(key, vkFB);
	return vkFB;
}

}

#endif
