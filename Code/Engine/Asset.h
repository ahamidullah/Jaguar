#pragma once

#include "Code/Basic/String.h"

#include "Model.h"
#include "Mesh.h"

void InitializeAssets(void *jobParameter);
MeshAsset *GetMeshAsset(String assetName);
bool IsAssetLoaded(AssetID assetID);
void FinalizeAssetUploadsToGPU();
