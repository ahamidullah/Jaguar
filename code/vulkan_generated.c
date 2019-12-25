// This file was auto-generated by material_compiler.c.

const char *SHADER_SPIRV_FILEPATHS[GPU_SHADER_COUNT][GPU_SHADER_STAGE_COUNT] = {
	[RUSTED_IRON_SHADER][GPU_VERTEX_SHADER_STAGE] = "build/shaders/binaries/rusted_iron_vert.spirv",
	[RUSTED_IRON_SHADER][GPU_FRAGMENT_SHADER_STAGE] = "build/shaders/binaries/rusted_iron_frag.spirv",
};

VkDescriptorPoolSize vulkan_descriptor_pool_sizes[] = {
	{
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1 + 1 * GPU_MAX_FRAMES_IN_FLIGHT,
	},
};

void Render_API_Load_Shaders(Render_API_Context *context, GPU_Shader shaders[GPU_SHADER_COUNT]) {
	for (s32 i = 0; i < GPU_SHADER_COUNT; i++) {
		for (s32 j = 0; j < GPU_SHADER_STAGE_COUNT; j++) {
			if (!SHADER_SPIRV_FILEPATHS[i][j]) {
				continue;
			}
			Read_File_Result spirv = Read_Entire_File(SHADER_SPIRV_FILEPATHS[i][j]);
			Assert(spirv.data);
			VkShaderModuleCreateInfo shader_module_create_info = {
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = spirv.size,
				.pCode = (u32 *)spirv.data,
			};
			shaders[i].stages[shaders[i].stage_count] = j;
			VK_CHECK(vkCreateShaderModule(context->device, &shader_module_create_info, NULL, &shaders[i].modules[shaders[i].stage_count]));
			shaders[i].stage_count++;
		}
	}
}

void Vulkan_Create_Descriptor_Sets(Render_API_Context *context, VkDescriptorPool descriptor_pool, GPU_Descriptor_Sets *descriptor_sets) {
	// Create the descriptor set layouts.
	{
		VkDescriptorSetLayoutBinding bindings[] = {
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.pImmutableSamplers = NULL,
			},
		};
		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = Array_Count(bindings),
			.pBindings = bindings,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
		};
		VK_CHECK(vkCreateDescriptorSetLayout(context->device, &descriptor_set_layout_create_info, NULL, &descriptor_sets->layouts.rusted_iron[RUSTED_IRON_VERTEX_BIND_PER_MATERIAL_UPDATE_DELAYED_DESCRIPTOR_SET]));
	}
	{
		VkDescriptorSetLayoutBinding bindings[] = {
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.pImmutableSamplers = NULL,
			},
		};
		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = Array_Count(bindings),
			.pBindings = bindings,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
		};
		VK_CHECK(vkCreateDescriptorSetLayout(context->device, &descriptor_set_layout_create_info, NULL, &descriptor_sets->layouts.rusted_iron[RUSTED_IRON_VERTEX_BIND_PER_OBJECT_UPDATE_IMMEDIATE_DESCRIPTOR_SET]));
	}

	// Create the descriptor sets.
	s32 layout_index = 0;
	VkDescriptorSetLayout layouts[GPU_DESCRIPTOR_SET_COUNT];
	layouts[layout_index++] = descriptor_sets->layouts.rusted_iron[RUSTED_IRON_VERTEX_BIND_PER_MATERIAL_UPDATE_DELAYED_DESCRIPTOR_SET];
	for (s32 i = 0; i != GPU_MAX_FRAMES_IN_FLIGHT; i++) {
		layouts[layout_index++] = descriptor_sets->layouts.rusted_iron[RUSTED_IRON_VERTEX_BIND_PER_OBJECT_UPDATE_IMMEDIATE_DESCRIPTOR_SET];
	}
	VkDescriptorSet results[GPU_DESCRIPTOR_SET_COUNT];
	VkDescriptorSetAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptor_pool,
		.descriptorSetCount = GPU_DESCRIPTOR_SET_COUNT,
		.pSetLayouts = layouts,
	};
	VK_CHECK(vkAllocateDescriptorSets(context->device, &allocate_info, results));
	s32 result_index = 0;
	descriptor_sets->rusted_iron_vertex_bind_per_material_update_delayed = results[result_index++];
	for (s32 i = 0; i < GPU_MAX_FRAMES_IN_FLIGHT; i++) {
		descriptor_sets->rusted_iron_vertex_bind_per_object_update_immediate[i] = results[result_index++];
	}
}

