typedef u32 Entity_ID;

#include "Mesh.h"

struct EntityMeshes {
	Array<MeshInstance> instances;
	Array<BoundingSphere> boundingSpheres;

	// Per-mesh data
	//Mesh_Render_Info render_info[MAX_ENITTY_MESHES];
	//Transform        transforms[MAX_ENITTY_MESHES];
	//Bounding_Sphere  bounding_spheres[MAX_ENITTY_MESHES];
	//u32              submesh_counts[MAX_ENITTY_MESHES];
	//u32              count;

	// Per-submesh data
	//u32      submesh_index_counts[MAX_ENITTY_SUBMESHES];
	//Material submesh_materials[MAX_ENITTY_SUBMESHES];
	//u32      submesh_count;
};

// @TODO: Use sparse arrays to store entity attributes.
struct GameEntities {
	Transform transforms[100]; // @TODO
	u32 transform_count;

	EntityMeshes meshes;
//	Array<ShaderEntityParameters> shaderParameters;

	u32 ids[100]; // @TODO
	u32 id_count;
} gameEntities;

u32 guy_id;
u32 nanosuit_id;

// @TODO: Multithreading?

Entity_ID Create_Entity() {
	Entity_ID id = gameEntities.id_count;
	gameEntities.ids[gameEntities.id_count++] = id;
	return id;
}

void Set_Entity_Transform(Entity_ID entity_id, Transform transform) {
	gameEntities.transforms[entity_id] = transform;
}

struct Set_Entity_Model_Job_Parameter {
	Asset_ID asset_id;
	Transform transform;
	EntityMeshes *meshes;
	Render_Context *render_context;
};

void Set_Entity_Model_Job(void *job_parameter_pointer) {
	Set_Entity_Model_Job_Parameter *job_parameter = (Set_Entity_Model_Job_Parameter *)job_parameter_pointer;
	MeshAsset *asset = GetMeshAsset(job_parameter->asset_id, job_parameter->render_context);
	Append(&job_parameter->meshes->instances, (MeshInstance){
		.transform = job_parameter->transform,
		.asset = asset,
	});
	Append(&job_parameter->meshes->boundingSpheres, (BoundingSphere){
		.center = asset->boundingSphere.center + job_parameter->transform.translation,
		.radius = asset->boundingSphere.radius,
	});
}

void Set_Entity_Model(Entity_ID entity_id, Asset_ID asset_id, Transform transform, GameState *game_state) {
	Set_Entity_Model_Job_Parameter *job_parameter = (Set_Entity_Model_Job_Parameter *)malloc(sizeof(Set_Entity_Model_Job_Parameter)); // @TODO
	*job_parameter = (Set_Entity_Model_Job_Parameter){
		.asset_id = asset_id,
		.transform = transform,
		.meshes = &gameEntities.meshes,
		.render_context = &game_state->render_context,
	};
	JobCounter *job_counter = (JobCounter *)malloc(sizeof(JobCounter)); // @TODO
	JobDeclaration job_declaration = CreateJob(Set_Entity_Model_Job, job_parameter);
	RunJobs(1, &job_declaration, NORMAL_PRIORITY_JOB, job_counter);
}

void InitializeEntities(GameState *game_state) {
	nanosuit_id = Create_Entity();
	Transform t = {.translation = {0.0f, 0.0f, 0.0f}};
	Set_Entity_Transform(nanosuit_id, t);
	Set_Entity_Model(nanosuit_id, ANVIL_ASSET, t, game_state);
}
