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

void set_entity_model(Game_Entities *entities, Entity_ID entity_id, Game_Assets *assets, Asset_ID asset_id, Transform transform) {
	Loaded_Mesh *asset = assets->lookup[asset_id];
	Mesh *mesh = &entities->meshes[entity_id];
	mesh->submesh_index_counts = asset->submesh_index_counts;
	mesh->vertex_offset = asset->vertex_offset;
	mesh->first_index = asset->first_index;

	entities->submesh_materials[entity_id] = asset->materials;

	entities->mesh_transforms[entity_id] = transform;

	//entities->submesh_uniform_block_indices[entity_id] = asset->submesh_uniform_block_indices;

	entities->submesh_counts[entity_id] = asset->submesh_count;

	entities->mesh_count++;
}

void initialize_entities(Game_Entities *entities, Game_Assets *assets) {
	entities->ids = malloc(sizeof(Entity_ID) * 10000);
	entities->transforms = malloc(sizeof(Transform) * 10000);
	entities->meshes = malloc(sizeof(Mesh) * 10000);
	entities->submesh_counts = malloc(sizeof(u32) * 10000);
	entities->submesh_uniform_block_indices = malloc(sizeof(u32) * 10000);
	entities->mesh_transforms = malloc(sizeof(Transform) * 10000);
	entities->submesh_materials = malloc(sizeof(Material *) * 10000);

	guy_id = create_entity(entities);
	Transform t = {};
	set_entity_transform(entities, guy_id, t);
	set_entity_model(entities, guy_id, assets, GUY1_ASSET, t);

	nanosuit_id = create_entity(entities);
	t.translation = (V3){5.0f, 0.0f, 0.0f};
	set_entity_transform(entities, guy_id, t);
	set_entity_model(entities, nanosuit_id, assets, NANOSUIT_ASSET, t);
}
