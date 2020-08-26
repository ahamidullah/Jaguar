#pragma once

#include "Basic/String.h"

#include "Model.h"
#include "Mesh.h"

enum AssetType
{
	AssetTypeModel,

	AssetTypeCount
};

enum AssetID
{
	AssetIDSponza,

	AssetIDCount
}

struct LoadAssetJobParameters
{
	AssetID id;
	AssetType type;
};

void InitializeAssets(void *jobParameter);
void LoadAsset(void *jobParams)
void UnloadAsset(AssetID id);
void *LockAsset(AssetID id);
void UnlockAsset(AssetID id);
String AssetIDToString();
