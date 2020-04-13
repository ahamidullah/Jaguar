struct DescriptorSets
{
	DescriptorSetGroup global;
	DescriptorSetGroup view;
	DescriptorSetGroup material;
	DescriptorSetGroup object;
};

struct DescriptorSetBindingInfo
{
	u32 binding;
	GfxDescriptorType descriptorType;
	u32 descriptorCount;
	GfxShaderStage stage;
};

DescriptorSetGroup CreateDescriptorSetGroup(DescriptorSetBindingInfo *bindingInfos, u32 swapchainImageCount);
