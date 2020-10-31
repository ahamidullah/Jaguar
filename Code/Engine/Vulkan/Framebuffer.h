#pragma once

#ifdef VulkanBuild

namespace GPU
{

struct Framebuffer
{
	u64 id;
	u32 width;
	u32 height;
	Array<VkImageView> attachments;
};

// @TODO
struct ImageView
{
	VkImageView vkImageView;
};

Framebuffer NewFramebuffer(u32 w, u32 h, ArrayView<ImageView> attachments);
Framebuffer DefaultFramebuffer();

VkFramebuffer NewVkFramebuffer(VkRenderPass rp, Framebuffer fb);

}

#endif
