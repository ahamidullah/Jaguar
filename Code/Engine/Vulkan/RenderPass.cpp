#ifdef VulkanBuild

#include "RenderPass.h"

namespace GPU
{

VkRenderPass NewRenderPass(String shaderFilename)
{
	if (shaderFilename == "Model.glsl")
	{
		auto attachments = MakeStaticArray(
			VkAttachmentDescription
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
			VkAttachmentDescription
			{
				.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			});
		auto colorAttachments = MakeStaticArray(
			VkAttachmentReference
			{
				.attachment = 0,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			});
		auto stencilAttachment = VkAttachmentReference
		{
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
		auto subpassDescs = MakeStaticArray(
			VkSubpassDescription
			{
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = (u32)colorAttachments.Count(),
				.pColorAttachments = colorAttachments.elements,
				.pDepthStencilAttachment = &stencilAttachment,
			});
		auto subpassDeps = MakeStaticArray(
			VkSubpassDependency
			{
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			});
		auto ci = VkRenderPassCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = (u32)attachments.Count(),
			.pAttachments = attachments.elements,
			.subpassCount = (u32)subpassDescs.Count(),
			.pSubpasses = subpassDescs.elements,
			.dependencyCount = (u32)subpassDeps.Count(),
			.pDependencies = subpassDeps.elements,
		};
		auto rp = VkRenderPass{};
		VkCheck(vkCreateRenderPass(vkDevice, &ci, NULL, &rp));
		return rp;
	}
	else
	{
		Abort("Vulkan", "Failed to make render pass: unknown shader %k.", shaderFilename);
	}
	return {};
}

}

#endif
