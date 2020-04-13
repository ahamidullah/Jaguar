#pragma once

struct RenderGraphResource
{
	String name;
	bool isTexture = false;
	bool isBuffer = false;
};

struct RenderPass
{
	String name;
	Shader *shader;
	Array<RenderGraphResource> read;
	Array<RenderGraphResource> write;
	Array<RenderGraphResource> create;
	void (*execute)(GfxCommandBuffer commandBuffer);
};

struct PhyisicalRenderPass
{
	GfxRenderPass pass;
	GfxPipeline pipeline;
};

struct RenderGraph
{
	bool compiled = false;
	Array<RenderPass> logicalPasses;
	Array<PhyisicalRenderPass> physicalPasses;
};

void AddRenderGraphPass(RenderGraph *graph, const RenderPass &pass);
void CompileRenderGraph(RenderGraph *graph);
void ExecuteRenderGraph(RenderGraph *graph);
