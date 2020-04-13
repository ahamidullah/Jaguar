#pragma once

enum AssetID
{
	GUY_ASSET,
	GUY2_ASSET,
	GUY3_ASSET,
	GUN_ASSET,
	ANVIL_ASSET,
	SPONZA_ASSET,
	ASSET_COUNT
};

enum AssetLoadStatus
{
	ASSET_UNLOADED,
	ASSET_LOADING,
	ASSET_LOADED,
};

struct MeshAsset;

void InitializeAssets(void *jobParameter);
MeshAsset *GetMeshAsset(AssetID assetID);
bool IsAssetLoaded(AssetID assetID);
void FinalizeAssetUploadsToGPU();
