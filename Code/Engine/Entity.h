#pragma once

typedef u32 EntityID;

EntityID CreateEntity();
void SetEntityTransform(EntityID entity_id, Transform transform);
void SetEntityModel(EntityID entity_id, AssetID asset_id, Transform transform);
