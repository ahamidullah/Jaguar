#pragma once

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

struct BoundingSphere
{
	V3 center;
	f32 radius;
};

struct Material
{
	union
	{
		struct
		{
			u32 albedo_map;
			u32 normal_map;
			u32 roughness_map;
			u32 metallic_map;
			u32 ambient_occlusion_map;
		};
		struct
		{
			V4 flat_color;
		};
	};
};

struct MeshAsset
{
	// @TODO: Seperate hot and cold data?
	// Hot. Potentially accessed every frame.
	AssetLoadStatus loadStatus;
	GPUIndexedGeometry gpuGeometry;
	struct Submesh
	{
		u32 indexCount;
		u32 firstIndex;
		u32 vertexOffset;
	};
	Array<Submesh> submeshes;
	Array<Material> materials;

	// Cold. Accessed when the asset is loaded.
	u32 vertex_count;
	u32 index_count;
	BoundingSphere boundingSphere;
};

struct MeshInstance
{
	Transform transform;
	MeshAsset *asset;
};

//@TODO: MeshAsset *LoadMeshAsset(const String &path);
