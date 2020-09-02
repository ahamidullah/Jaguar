#pragma once

#include "Model.h"
#include "Mesh.h"
#include "Basic/String.h"

enum AssetType
{
	ModelAssetType,
	AssetTypeCount
};

enum AssetID
{
	SponzaAssetID,
	AssetIDCount
};

struct LoadAssetJobParameters
{
	AssetID id;
	AssetType type;
};

void InitializeAssets(void *jobParameter);
void LoadAsset(AssetID id);
void UnloadAsset(AssetID id);
void *LockAsset(AssetID id);
void UnlockAsset(AssetID id);
String AssetIDToString();
