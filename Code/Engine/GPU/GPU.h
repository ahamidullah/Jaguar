#pragma once

namespace GPU
{

struct GPU
{
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;
	virtual MeshAsset NewMeshAsset() = 0;
	virtual Mesh NewMesh() = 0;
	virtual Material NewMaterial() = 0;
	virtual StagingBuffer NewStagingBuffer() = 0;
	virtual void NewCommandBuffer() = 0;
	virtual FrameBuffer DefaultFrameBuffer() = 0;
	virtual Shader CompileShader() = 0;
	virtual void SubmitTransferCommands() = 0;
	virtual void SubmitGraphicsCommands() = 0;
};

GPU New();

};
