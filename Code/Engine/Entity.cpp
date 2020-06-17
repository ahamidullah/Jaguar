#include "Entity.h"
#include "Mesh.h"
#include "Job.h"

struct EntityMeshes
{
	Array<MeshInstance> instances;
	Array<BoundingSphere> boundingSpheres;
};

template <typename T>
struct EntityComponentList
{
	// SpinLock spinLock;
	// BucketArray<T> components;
};

struct Entities
{
	u64 count;
	// EntityComponentList<Transform> transforms;
	Array<Transform> transforms;
	Array<ModelInstance> models;
	Array<MeshInstance> meshes;
} entityGlobals;

// @TODO: Use sparse arrays to store entity attributes.
struct
{
	Transform transforms[100]; // @TODO
	s64 transform_count;

	EntityMeshes meshes;

	s64 ids[100]; // @TODO
	s64 id_count;
} entitiesContext;

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
	String modelName;
	Transform transform;
};

void Set_Entity_Model_Job(void *job_parameter_pointer)
{
	Set_Entity_Model_Job_Parameter *job_parameter = (Set_Entity_Model_Job_Parameter *)job_parameter_pointer;
	MeshAsset *asset = GetMeshAsset(job_parameter->modelName);
	ArrayAppend(&entitiesContext.meshes.instances,
	            {
	            	.transform = job_parameter->transform,
	            	.asset = asset,
	            });
	ArrayAppend(&entitiesContext.meshes.boundingSpheres,
	            {
	            	.center = asset->boundingSphere.center + job_parameter->transform.position,
	            	.radius = asset->boundingSphere.radius,
	            });
}

void SetEntityModel(EntityID entity_id, String modelName, Transform transform)
{
	Set_Entity_Model_Job_Parameter *job_parameter = (Set_Entity_Model_Job_Parameter *)malloc(sizeof(Set_Entity_Model_Job_Parameter)); // @TODO
	*job_parameter = (Set_Entity_Model_Job_Parameter){
		.modelName = modelName,
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
}
