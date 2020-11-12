#pragma once

#include "Mesh.h"
#include "Basic/String.h"

struct ModelAsset
{
	MeshAsset *mesh;
	//Skeleton *skeleton;

	#ifdef DebugBuild
		string::String name;
	#endif
};

struct Model
{
	//String modelName;
	Mesh mesh;
	//Skeleton *skeleton;
};

void InitializeModelAssets();
ModelAsset LoadModelAsset(string::String name);
