#pragma once

#include "Mesh.h"

struct ModelAsset
{
	String name;
	String meshAssetName;
	AssetLoadStatus loadStatus;
};

struct ModelInstance
{
	String assetName;
};
