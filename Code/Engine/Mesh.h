#pragma once

#if 0

#include "Asset.h"
#include "Math.h"
#include "Transform.h"
#include "GPU.h"

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

struct MeshAsset
{
	GPUIndexedGeometry gpuGeometry;
	struct Submesh
	{
		u32 indexCount;
		u32 firstIndex;
		u32 vertexOffset;
	};
	Array<Submesh> submeshes;
};

struct MeshInstance
{
	String assetName;
};

#endif
