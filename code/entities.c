typedef u32 Entity_ID;

u32 guy_id;
u32 nanosuit_id;

Entity_ID Create_Entity(Game_Entities *entities) {
	Entity_ID id = entities->id_count;
	entities->ids[entities->id_count++] = id;
	return id;
}

void Set_Entity_Transform(Entity_ID entity_id, Transform transform, Game_Entities *entities) {
	entities->transforms[entity_id] = transform;
}

typedef struct Finalize_Set_Entity_Model_Job_Parameter {
	Mesh_Asset **asset;
	Job_Counter *load_mesh_job_counter;
	Transform transform;
	Entity_Meshes *meshes;
	Entity_ID entity_id;
} Finalize_Set_Entity_Model_Job_Parameter;

//typedef struct Model_Asset {
	// Skeleton *skeleton;
	// Render_Mesh_Asset *render_mesh;
	// Collision_Mesh_Asset *collision_mesh;
	// Material_Asset *materials;
//} Model_Asset;

/*
void Finalize_Set_Entity_Model(void *job_parameter_pointer) {
	Finalize_Set_Entity_Model_Job_Parameter *job_parameter = (Finalize_Set_Entity_Model_Job_Parameter *)job_parameter_pointer;
	Wait_For_Counter(job_parameter->load_mesh_job_counter);
	Mesh_Asset *asset = *job_parameter->asset;
	//job_parameter->meshes->instances[job_parameter->entity_id] = (Mesh_Instance){
	job_parameter->meshes->instances[job_parameter->meshes->count] = (Mesh_Instance){
		.transform = job_parameter->transform,
		.asset = asset,
	};
	//job_parameter->meshes->bounding_spheres[job_parameter->entity_id] = (Bounding_Sphere){
	job_parameter->meshes->bounding_spheres[job_parameter->meshes->count] = (Bounding_Sphere){
		.center = add_v3(asset->bounding_sphere.center, job_parameter->transform.translation),
		.radius = asset->bounding_sphere.radius,
	};
	job_parameter->meshes->count++;
	Assert(job_parameter->meshes->count < MAX_ENTITY_MESHES);
}
*/

typedef struct Set_Entity_Model_Job_Parameter {
	Asset_ID asset_id;
	Transform transform;
	Game_Assets *assets;
	Entity_Meshes *meshes;
	GPU_Context *gpu_context;
	GPU_Upload_Flags gpu_upload_flags;
} Set_Entity_Model_Job_Parameter;

void Set_Entity_Model_Job(void *job_parameter_pointer) {
	Set_Entity_Model_Job_Parameter *job_parameter = job_parameter_pointer;
	Job_Counter job_counter;
	Mesh_Asset **asset = Get_Mesh_Asset(job_parameter->asset_id, job_parameter->assets, job_parameter->gpu_context, job_parameter->gpu_upload_flags, &job_counter);
	Wait_For_Job_Counter(&job_counter);
	job_parameter->meshes->instances[job_parameter->meshes->count] = (Mesh_Instance){
		.transform = job_parameter->transform,
		.asset = *asset,
	};
	job_parameter->meshes->bounding_spheres[job_parameter->meshes->count] = (Bounding_Sphere){
		.center = add_v3((*asset)->bounding_sphere.center, job_parameter->transform.translation),
		.radius = (*asset)->bounding_sphere.radius,
	};
	job_parameter->meshes->count++;
	Assert(job_parameter->meshes->count < MAX_ENTITY_MESHES);
}

void Set_Entity_Model(Entity_ID entity_id, Asset_ID asset_id, Transform transform, GPU_Upload_Flags gpu_upload_flags, Game_State *game_state) {
	Set_Entity_Model_Job_Parameter *job_parameter = malloc(sizeof(Set_Entity_Model_Job_Parameter)); // @TODO
	*job_parameter = (Set_Entity_Model_Job_Parameter){
		.asset_id = asset_id,
		.transform = transform,
		.assets = &game_state->assets,
		.meshes = &game_state->entities.meshes,
		.gpu_context = &game_state->render_context.gpu_context,
		.gpu_upload_flags = gpu_upload_flags,
	};
	Job_Counter *job_counter = malloc(sizeof(Job_Counter)); // @TODO
	Job_Declaration job_declaration = Create_Job(Set_Entity_Model_Job, job_parameter);
	Run_Jobs(1, &job_declaration, NORMAL_JOB_PRIORITY, job_counter);
}

void Initialize_Entities(Game_State *game_state) {
	nanosuit_id = Create_Entity(&game_state->entities);
	Transform t = {.translation = {0.0f, 0.0f, 0.0f}};
	Set_Entity_Transform(nanosuit_id, t, &game_state->entities);
	Set_Entity_Model(nanosuit_id, ANVIL_ASSET, t, 0, game_state);

/*
	t = (Transform){.translation = {3.264736, 1.547395, 1.265500}};
	guy_id = Create_Entity(&game_state->entities);
	Set_Entity_Transform(guy_id, t, &game_state->entities);
	Set_Entity_Model(guy_id, GUY_ASSET, t, 0, game_state);
	*/
}
