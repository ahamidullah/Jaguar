#pragma once

#include "GPU.h"
#include "Math.h"
#include "Transform.h"

struct Vertex1P1C1UV1N1T
{
	V3 position;
	V3 color;
	V2 uv;
	V3 normal;
	V3 tangent;
};

struct Vertex1P
{
	V3 position;
};

struct Vertex1P1N
{
	V3 position;
	V3 normal;
};

struct SubmeshAsset
{
	u32 indexCount;
	u32 firstIndex;
	u32 vertexOffset;
};

struct MeshAsset
{
	u32 indexCount;
	GPUIndexType indexType;
#if OLD_VULKAN_BUFFER
	GPUBuffer vertexBuffer;
	GPUBuffer indexBuffer;
#else
	GPUBufferX vertexBuffer;
	GPUBufferX indexBuffer;
	s64 vertexOffset;
	s64 firstIndex;
#endif
	//GPUIndexedGeometry gpuGeometry;
	//Array<SubmeshAsset> submeshes;
};

struct Mesh
{
	String meshName;
};
