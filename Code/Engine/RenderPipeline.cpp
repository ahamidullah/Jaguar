void CreateGraphicsPipeline(Array<GfxPipeline> *pipelines, const Array<DescriptorSetGroup> &sets)
{
	Array<GfxDescriptorSetLayout> layouts;
	for (const auto &set : sets)
	{
		Append(&layouts, set.layout);
	}

	GfxFramebufferAttachmentColorBlendDescription colorBlendDescription =
	{
		.enable_blend = true,
		.source_color_blend_factor = GFX_BLEND_FACTOR_SRC_ALPHA,
		.destination_color_blend_factor = GFX_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		//.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		//.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.color_blend_operation = GFX_BLEND_OP_ADD ,
		.source_alpha_blend_factor = GFX_BLEND_FACTOR_ONE,
		.destination_alpha_blend_factor = GFX_BLEND_FACTOR_ZERO,
		.alpha_blend_operation = GFX_BLEND_OP_ADD,
		.color_write_mask = GFX_COLOR_COMPONENT_RED | GFX_COLOR_COMPONENT_GREEN | GFX_COLOR_COMPONENT_BLUE | GFX_COLOR_COMPONENT_ALPHA,
	};
	GfxPipelineVertexInputBindingDescription vertexInputBindingDescriptions[] =
	{
		{
			.binding = GFX_VERTEX_BUFFER_BIND_ID,
			.stride = sizeof(Vertex1P1N),
			.input_rate = GFX_VERTEX_INPUT_RATE_VERTEX,
		},
	};
	GfxPipelineVertexInputAttributeDescription vertexInputAttributeDescriptions[] =
	{
		{
			.format = GFX_FORMAT_R32G32B32_SFLOAT,
			.binding = GFX_VERTEX_BUFFER_BIND_ID,
			.location = 0,
			.offset = offsetof(Vertex1P1N, position),
		},
		{
			.format = GFX_FORMAT_R32G32B32_SFLOAT,
			.binding = GFX_VERTEX_BUFFER_BIND_ID,
			.location = 1,
			.offset = offsetof(Vertex1P1N, normal),
		},
	};
	GfxDynamicPipelineState dynamicStates[] =
	{
		GFX_DYNAMIC_PIPELINE_STATE_VIEWPORT,
		GFX_DYNAMIC_PIPELINE_STATE_SCISSOR,
	};
	auto shader = GetShader("scene");
	Assert(shader != NULL);
	auto renderPass = GetRenderPass("lighting");
	Assert(renderPass != NULL);
	GfxPipelineDescription pipelineDescription =
	{
		.descriptor_set_layout_count = Length(layouts),
		.descriptor_set_layouts = &layouts[0],
		.push_constant_count = 0,
		.push_constant_descriptions = NULL,
		.topology = GFX_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.viewport_width = (f32)windowWidth,
		.viewport_height = (f32)windowHeight,
		.scissor_width = windowWidth,
		.scissor_height = windowHeight,
		.depth_compare_operation = GFX_COMPARE_OP_LESS,
		.framebuffer_attachment_color_blend_count = 1,
		.framebuffer_attachment_color_blend_descriptions = &colorBlendDescription,
		.vertex_input_attribute_count = ArrayCount(vertexInputAttributeDescriptions),
		.vertex_input_attribute_descriptions = vertexInputAttributeDescriptions,
		.vertex_input_binding_count = ArrayCount(vertexInputBindingDescriptions),
		.vertex_input_binding_descriptions = vertexInputBindingDescriptions,
		.dynamic_state_count = ArrayCount(dynamicStates),
		.dynamic_states = dynamicStates,
		.shader = shader,
		.render_pass = renderPass,
		.enable_depth_bias = false,
	};
	pipelines[0] = GfxCreatePipeline(&pipelineDescription); // @TODO
}
