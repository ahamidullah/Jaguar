#pragma once

#ifdef VulkanBuild

#include "Swapchain.h"

namespace GPU::Vulkan
{

struct Framebuffer
{
	u64 id;
	u32 width;
	u32 height;
	Array<VkImageView> attachments;
	Swapchain *swapchain;
	Device *device;
};

// @TODO
struct ImageView
{
	VkImageView vkImageView;
};

Framebuffer NewFramebuffer(Swapchain *sc, Device *d, u32 w, u32 h, ArrayView<ImageView> attachments);
Framebuffer DefaultFramebuffer(Swapchain *sc, Device *d);

VkFramebuffer NewVkFramebuffer(VkRenderPass rp, Framebuffer fb);

}

#endif
