#ifdef VulkanBuild

namespace GPU::Vulkan
{

Framebuffer NewFramebuffer(Swapchain *sc, Device *d, u32 w, u32 h, array::View<ImageView> attachments)
{
	static auto idGenerator = u64{0};
	auto id = (u64)AtomicFetchAndAdd64((s64 *)&idGenerator, 1);
	// The first few ids are reserved for the default swapchain framebuffer.
	if (id < sc->images.count)
	{
		id = (u64)AtomicFetchAndAdd64((s64 *)&idGenerator, sc->images.count);
	}
	auto fb = Framebuffer
	{
		.id = id,
		.width = w,
		.height = h,
		.attachments = array::NewWithCapacityIn<VkImageView>(Memory::GlobalHeap(), attachments.count),
		.swapchain = sc,
		.device = d,
	};
	for (auto a : attachments)
	{
		fb.attachments.Append(a.vkImageView);
	}
	return fb;
}

Framebuffer DefaultFramebuffer(Swapchain *sc, Device *d)
{
	return
	{
		.id = 0,
		.swapchain = sc,
		.device = d,
	};
}

struct FramebufferKey
{
	VkRenderPass renderPass;
	u64 framebufferID;

	bool operator==(FramebufferKey v);
};

bool FramebufferKey::operator==(FramebufferKey k)
{
	return this->renderPass == k.renderPass && this->framebufferID == k.framebufferID;
}

u64 HashFramebufferKey(FramebufferKey k)
{
	return HashPointer(k.renderPass) ^ Hash64(k.framebufferID);
}

auto framebufferCache = map::New<FramebufferKey, VkFramebuffer>(0, HashFramebufferKey);

VkFramebuffer NewVkFramebuffer(VkRenderPass rp, Framebuffer fb)
{
	if (fb.id == 0)
	{
		// Default framebuffer.
		fb.id = fb.swapchain->imageIndex;
		fb.width = u32(RenderWidth());
		fb.height = u32(RenderHeight());
		fb.attachments = array::Make<VkImageView>(fb.swapchain->imageViews[fb.swapchain->imageIndex], fb.swapchain->defaultDepthImageView);
	}
	auto key = FramebufferKey
	{
		.renderPass = rp,
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
		.attachmentCount = u32(fb.attachments.count),
		.pAttachments = fb.attachments.elements,
		.width = fb.width,
		.height = fb.height,
		.layers = 1,
	};
	auto vkFB = VkFramebuffer{};
	Check(vkCreateFramebuffer(fb.device->device, &ci, NULL, &vkFB));
	framebufferCache.Insert(key, vkFB);
	return vkFB;
}

}

#endif
