#pragma once

#if 0

#include "Mesh.h"

struct ModelAsset
{
	MeshAsset *mesh;
	SkeletonAsset *skeleton;

	#if DEBUG_BUILD
		String name;
	#endif
};

struct ModelInstance
{
	String assetName;
	MeshInstance mesh;
	SkeletonInstance *skeleton;
};

#endif
