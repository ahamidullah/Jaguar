#pragma once

#include "Asset.h"
#include "Transform.h"

#include "Common.h"

void InitializeEntities();
u64 NewEntity();
void SetEntityTransform(u64 id, Transform t);
void SetEntityModel(u64 id, AssetID asset);
