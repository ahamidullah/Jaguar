void AddRenderGraphPass(RenderGraph *graph, const RenderPass &pass)
{
	Append(&graph->passes, pass);
}

void CompileRenderGraph(RenderGraph *graph)
{
	graph->compiled = true;
	for (const auto &logicalPass : graph->logicalPasses)
	{
		Append(&graph->apiPasses,
		{
			.gfxPass = TEMPORARY_Render_API_Create_Render_Pass(),
			.gfxPipeline = CreateGraphicsPipeline(logicalPass.shader->GetGraphicsPipelineCreationInfo()),
		});
	}
}

GfxCommandBuffer ExecuteRenderGraph(RenderGraph *graph, u32 swapchainImageIndex)
{
	for (const auto &pass : graph->physicalPasses)
	{
		auto swapchainImageIndex = GetSwapchainImageIndex();
		auto commandBuffer = CreateGraphicsCommandBuffer();
		GfxRecordBeginRenderPassCommand(commandBuffer, pass.gfxPass, framebuffers[swapchainImageIndex]);
		GfxRecordBindPipelineCommand(commandBuffer, pass.gfxPipeline);
		pass.execute(commandBuffer);
		return commandBuffer;
		//SubmitCommandBuffer(commandBuffer);
	}
}
