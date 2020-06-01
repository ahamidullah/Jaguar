#pragma once

#include "Asset.h"
#include "Transform.h"

#include "Code/Common.h"

typedef u32 EntityID;

void InitializeEntities();
EntityID CreateEntity();
void SetEntityTransform(EntityID entity_id, Transform transform);
void SetEntityModel(EntityID entity_id, AssetID asset_id, Transform transform);
