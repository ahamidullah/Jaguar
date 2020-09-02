#include "Entity.h"

void SetEntityTransform(u64 id, Transform t)
{
}

void SetEntityModel(u64 id, AssetID asset)
{
}

void InitializeEntities()
{
}

u64 NewEntity()
{
	return 0;
}

#if 0
#include "Entity.h"
#include "Mesh.h"
#include "Job.h"

const auto EntityComponentIndexNotFound = s64{-1};

enum ComponentType
{
	ComponentTypeTransform,
	ComponentTypeMesh,

	ComponentTypeCount
};

template <typename T>
struct ComponentList
{
	Spinlock lock;
	HashTable<u64, s64> idToIndex;
	Array<u64> indexToID;
	Array<T> components;

	void SetAll(ArrayView<u64> ids, ArrayView<T> cs);
	void Set(u64 id, T c);
	void GetAll(ArrayView<u64> ids, Array<T *> *out);
	void GetAllStatic(ArrayView<u64> ids, StaticArray<T *, N> *out);
	T *Get(u64 id);
	void RemoveAll(ArrayView<u64> ids);
	void Remove(u64 id);
};

struct Entities
{
	u64 idGenerator;
	ComponentList<Transform> transforms;
	ComponentList<MeshInstance> meshes;
} entities;

template <typename T>
ComponentList<T> NewComponentList()
{
	return
	{
		.idToIndex = NewHashTableWithCount<u64, s64>(GlobalHeapAllocator(), 0),
		.indexToID = NewArrayWithCount<s64>(GlobalHeapAllocator(), 0),
		.components = NewArrayWithCount<T>(GlobalHeapAllocator(), 0),
	};
}

template <typename T>
void ComponentList<T>::SetAll(ArrayView<u64> ids, ArrayView<T> cs)
{
	l->lock.Lock();
	Defer(l->lock.Unlock());
	l->idToIndex.Reserve(l->idToIndex.count + ids.count);
	for (auto i = 0; i < ids.count; i++)
	{
		l->idToIndex.Insert(ids[i], l->indexToID.count + i);
	}
	l->indexToID.AppendAll(ids);
	l->components.AppendAll(cs);
}

template <typename T>
void ComponentList<T>::Set(u64 id, T c)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	this->indexToID.Append(id);
	auto index = this->indexToID.count;
	this->idToIndex.Insert(id, index);
	this->components.Append(c);
}

template <typename T>
void ComponentList<T>::GetAll(ArrayView<u64> ids, Array<T *> *out)
{
	out->Reserve(out->count + ids.count);
	this->lock.Lock();
	Defer(this->lock.Unlock());
	for (auto id : ids)
	{
		out->Append(this->idToIndex.LookupPointer(id));
	}
}

template <typename T, s64 N>
void ComponentList<T>::GetAllStatic(ArrayView<u64> ids, StaticArray<T *, N> *out)
{
	this->lock.Lock();
	Defer(this->lock.Unlock());
	for (auto i = 0; i < ids.count; i++)
	{
		(*out)[i] = this->idToIndex.LookupPointer(ids[i]);
	}
}

template <typename T>
T *ComponentList<T>::Get(ComponentList<T> *l, u64 id)
{
	l->lock.Lock();
	Defer(l->lock.Unlock());
	return l->components.LookupPointer(id);
}

template <typename T>
void ComponentList<T>::RemoveAll(ArrayView<u64> ids)
{
	auto allIndices = NewArrayWithCount<s64>(ContextAllocator(), ids.count);
	this->lock.Lock();
	Defer(
	{
		this->lock.Unlock();
		allIndices.Free();
	});
	for (auto i = 0; i < ids.count; i++)
	{
		allIndices[i] = this->idToIndex.Lookup(ids, EntityComponentIndexNotFound);
	}
	auto indices = Sift(&allIndices, [](s64 v)
	{
		if (v == EntityComponentIndexNotFound)
		{
			return false;
		}
		return true;
	}; 
	for (auto i : indices)
	{
		this->components.UnorderedRemove(i);
	}
	// We need to fix up the entity ID to index mappings after the unordered removal of components.
	for (auto i : indices)
	{
		this->indexToID.UnorderedRemove(i);
	}
	for (auto id : ids)
	{
		this->idToIndex.Remove(id);
	}
	for (auto i : indices)
	{
		this->idToIndex.Insert(this->indexToID[i], i);
	}
}

template <typename T>
void ComponentList<T>::Remove(u64 id)
{
	l->lock.Lock();
	Defer(l->lock.Unlock());
	auto i = l->idToIndex.Lookup(id, EntityComponentIndexNotFound);
	if (i == EntityComponentIndexNotFound)
	{
		return;
	}
	l->components.UnorderedRemove(i);
	l->indexToID.UnorderedRemove(i);
	l->idToIndex.Remove(id);
	l->idToIndex.Insert(l->indexToID[i], i);
}

void InitializeEntities()
{
	for (auto i = 0; i < ComponentTypeCount; i++)
	{
		auto e = (ComponentType)i;
		switch (e)
		{
		case ComponentTypeTransform:
		{
			entities.transforms = NewComponentList<Transform>();
		} break;
		case ComponentTypeMesh:
		{
			entities.meshes = NewComponentList<MeshInstance>();
		} break;
		}
	}
}

void NewEntities(s64 count, Array<u64> *out)
{
	// @TODO: Switch this to random number generation to aid in serialization.
	for (auto i = 0; i < count; i++)
	{
		Append(out, AtomicFetchAndAdd(&entities.idGenerator, 1));
	}
}

u64 NewEntity()
{
	// @TODO: Switch this to random number generation to aid in serialization.
	return AtomicFetchAndAdd(&entities.idGenerator, 1);
}

void DeleteEntities(ArrayView<u64> ids)
{
	for (auto i = 0; i < ComponentTypeCount; i++)
	{
		auto e = (AssetType)i;
		switch (e)
		{
		case ComponentTypeTransform:
		{
			entities.transforms.RemoveAll(ids);
		} break;
		case ComponentTypeMesh:
		{
			entities.meshes.RemoveAll(ids);
		} break;
		default:
		{
			Abort("Entity", "Unknown component type: %d.\n", ct);
		} break;
		}
	}
}

void DeleteEntity(u64 id)
{
	for (auto i = 0; i < ComponentTypeCount; i++)
	{
		auto e = (AssetType)i;
		switch (e)
		{
		case ComponentTypeTransform:
		{
			entities.transforms.Remove(id);
		} break;
		case ComponentTypeMesh:
		{
			entities.meshes.Remove(id);
		} break;
		default:
		{
			Abort("Entity", "Unknown component type: %d.\n", i);
		} break;
		}
	}
}

void SetEntityTransforms(ArrayView<u64> ids, ArrayView<Transform> ts)
{
	entities.transforms.SetAll(ids, ts);
}

void SetEntityTransform(u64 id, Transform t)
{
	entities.transforms.Set(id, t);
}

void EntityTransforms(ArrayView<u64> ids, Array<Transform *> *out)
{
	entities.transforms.GetAll(ids, out);
}

template <s64 N>
void EntityTransformsStatic(ArrayView<u64> ids, StaticArray<Transform *, N> *out)
{
	entities.transforms.GetAllStatic(ids, out);
}

Transform *EntityTransform(u64 id)
{
	return entities.transforms.Get(id);
}

void SetEntityModelComponents(u64 id, ModelAsset *m)
{
	entities.meshes.Set(id, MeshInstance{m->mesh});
}

void SetEntityModels(ArrayView<u64> ids, ArrayView<AssetID> models)
{
	for (auto i = 0; i < ids.count; i++)
	{
		SetEntityModel(ids[i], models[i]);
	}
}

void SetEntityModel(u64 id, AssetID model)
{
	LogVerbose("Entity", "Entity %lu: setting model %k(%d).\n", id, AssetIDToString(model), model);
	auto s = AssetStatus{};
	auto m = LockAsset(model, &s);
	if (s == AssetStatusLoaded)
	{
		SetEntityModelComponents(id, m);
		return;
	}
	else if (s == AssetStatusLoading)
	{
		LogVerbose("Entity", "Entity %lu: waiting for model %k(%d) to finish loading.\n", id, AssetIDToString(model), model);
		struct SetEntityModelJobParameter
		{
			void *modelID;
			u64 entityID;
			ModelAsset *model;
		};
		auto jp = (SetEntityModelJobParameter *)GlobalHeapAllocator().allocateMemory(GlobalHeapAllocator().data, sizeof(SetEntityModelJobParameter));
		jp->modelID = model;
		jp->entityID = id;
		jp->model = m;
		auto j = NewStaticArray<JobDeclaration>(NewJob(
			[](void *jobParams)
			{
				auto p = (SetEntityModelJobParameter *)jobParams;
				WaitForAssetLoad(p->modelID);
				SetEntityModelComponents(p->entityID, p->model);
				FreeMemory(p);
			},
			jp));
		RunJobs(j, NoJobCounter);
	}
	else if (s == AssetStatusNotResident)
	{
		LogError("Entity", "Entity %lu: failed to set model: %k(%d) is not resident.\n", id, AssetIDToString(model), model);
		return;
	}
}

void *EntityModels(ArrayView<u64> ids, Array<ModelAsset> *out)
{
	return entities.model.GetAll(ids, out);
}

void *EntityModelsStatic(ArrayView<u64> ids, StaticArray<ModelAsset> *out)
{
	return entities.model.GetAllStatic(ids, out);
}

ModelAsset *EntityModel(u64 id)
{
	return entities.models.Get(id);
}

Array<MeshInstance> MeshInstances()
{
	return entities.meshes.components;
}
#endif
