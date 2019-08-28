typedef u32 Entity_ID;

u32 guy_id;
u32 nanosuit_id;

Entity_ID create_entity(Game_Entities *entities) {
	Entity_ID id = entities->id_count;
	entities->ids[entities->id_count++] = id;
	return id;
}

void set_entity_transform(Game_Entities *entities, Entity_ID entity_id, Transform transform) {
	entities->transforms[entity_id] = transform;
}

void set_entity_model(Game_Entities *entities, Entity_ID entity_id, Game_Assets *assets, Asset_ID asset_id, Transform transform, Memory_Arena *arena) {
	Mesh_Asset *asset = get_mesh_asset(asset_id, assets, arena);
	entities->meshes.instances[entity_id] = (Mesh_Instance){
		.transform            = transform,
		.submesh_material_ids = asset->materials,
		.submesh_count        = asset->submesh_count,
		.vertex_offset        = asset->vertex_offset,
		.first_index          = asset->first_index,
		.submesh_index_counts = asset->submesh_index_counts,
	};
	entities->meshes.bounding_spheres[entity_id] = (Bounding_Sphere){
		.center = add_v3(asset->bounding_sphere.center, transform.translation),
		.radius = asset->bounding_sphere.radius,
	};
	entities->meshes.count++;
	ASSERT(entities->meshes.count < MAX_ENTITY_MESHES);
}

void initialize_entities(Game_Entities *entities, Game_Assets *assets, Memory_Arena *arena) {
	nanosuit_id = create_entity(entities);
	Transform t = {.translation = {0.0f, 0.0f, 0.0f}};
	set_entity_transform(entities, nanosuit_id, t);
	set_entity_model(entities, nanosuit_id, assets, ANVIL_ASSET, t, arena);

	t = (Transform){.translation = {3.264736, 1.547395, 1.265500}};
	guy_id = create_entity(entities);
	set_entity_transform(entities, guy_id, t);
	set_entity_model(entities, guy_id, assets, GUY_ASSET, t, arena);
}
