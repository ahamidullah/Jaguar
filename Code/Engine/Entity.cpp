struct EntityMeshes
{
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
struct
{
	Transform transforms[100]; // @TODO
	u32 transform_count;

	EntityMeshes meshes;

	u32 ids[100]; // @TODO
	u32 id_count;
} entitiesContext;

u32 guy_id;
u32 nanosuit_id;

// @TODO: Multithreading?

EntityID CreateEntity()
{
	auto id = entitiesContext.id_count;
	entitiesContext.ids[entitiesContext.id_count++] = id;
	return id;
}

void SetEntityTransform(EntityID entity_id, Transform transform)
{
	entitiesContext.transforms[entity_id] = transform;
}

struct Set_Entity_Model_Job_Parameter
{
	AssetID asset_id;
	Transform transform;
};

void Set_Entity_Model_Job(void *job_parameter_pointer)
{
	Set_Entity_Model_Job_Parameter *job_parameter = (Set_Entity_Model_Job_Parameter *)job_parameter_pointer;
	MeshAsset *asset = GetMeshAsset(job_parameter->asset_id);
	Append(&entitiesContext.meshes.instances, (MeshInstance){
		.transform = job_parameter->transform,
		.asset = asset,
	});
	Append(&entitiesContext.meshes.boundingSpheres, (BoundingSphere){
		.center = asset->boundingSphere.center + job_parameter->transform.translation,
		.radius = asset->boundingSphere.radius,
	});
}

void SetEntityModel(EntityID entity_id, AssetID asset_id, Transform transform)
{
	Set_Entity_Model_Job_Parameter *job_parameter = (Set_Entity_Model_Job_Parameter *)malloc(sizeof(Set_Entity_Model_Job_Parameter)); // @TODO
	*job_parameter = (Set_Entity_Model_Job_Parameter){
		.asset_id = asset_id,
		.transform = transform,
	};
	JobCounter *job_counter = (JobCounter *)malloc(sizeof(JobCounter)); // @TODO
	JobDeclaration job_declaration = CreateJob(Set_Entity_Model_Job, job_parameter);
	RunJobs(1, &job_declaration, NORMAL_PRIORITY_JOB, job_counter);
}

Array<MeshInstance> GetMeshInstances()
{
	return entitiesContext.meshes.instances;
}

void InitializeEntities()
{
/*
	nanosuit_id = CreateEntity();
	Transform t = {.translation = {0.0f, 0.0f, 0.0f}};
	SetEntityTransform(nanosuit_id, t);
	SetEntityModel(nanosuit_id, ANVIL_ASSET, t);
*/
}
