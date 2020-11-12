#include "Asset.h"

#include "Asset.h"
//#define STBI_MALLOC AllocateMemory
//#define STBI_REALLOC ResizeMemory
//#define STBI_FREE DeallocateMemory
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#define TINYGLTF_NOEXCEPTION
//#define TINYGLTF_IMPLEMENTATION
////#include "tiny_gltf.h"
//#undef TINYGLTF_IMPLEMENTATION
//#undef STB_IMAGE_IMPLEMENTATION
//#undef STB_IMAGE_WRITE_IMPLEMENTATION

void InitializeAssets(void *)
{
	InitializeModelAssets();
}

void LoadAsset(String name)
{
	LoadModelAsset(name);
}

#if 0
#include "Asset.h"
#include "GPU.h"
#include "PCH.h"
#include "Mesh.h"
#include "Job.h"
#include "Basic/Log.h"
#include "Basic/Filesystem.h"
#include "Basic/HashTable.h"
#include "Basic/Hash.h"
#define STBI_MALLOC AllocateMemory
#define STBI_REALLOC ResizeMemory
#define STBI_Free DeallocateMemory
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"
#undef TINYGLTF_IMPLEMENTATION

// Subassets can be referenced by assets (including other Subassets), but are not usable outside of the asset system.
template <typename T>
struct Subasset
{
	T data;
	s64 refCount;
}

struct AssetContainer
{
	Spinlock lock;
	void *asset;
	AssetType type;
	bool resident;
	bool loading;
	s64 loadCount;
	s64 lockCount;
	#if DebugBuild
		String name;
	#endif
};

Spinlock assetSlotsLock;
SlotAllocator assetSlots[AssetTypeCount];
array::Static<AssetContainer, AssetIDCount> assetContainers;

void InitializeAssets(void *)
{
	for (auto i = 0; i < AssetTypeCount; i++)
	{
		switch (i)
		{
		case ModelAsset:
		{
			assetSlots[i] = NewSlotAllocator(KilobytesToBytes(64), 0, GlobalHeap(), GlobalHeap());
		} break;
		default:
		{
			Abort("Unknown asset type %d.", i);
		} break;
		}
	}
}

// @TODO: Trying to load the same asset multiple times simultaneously?

GPUCommandBuffer theBuffer;
bool ready = false;

GPUIndexedGeometry QueueIndexedGeometryUploadToGPU(s64 vertByteSize, s64 idxByteSize, GfxBuffer staging)
{
	auto vb = NewGPUBuffer(vertByteSize, GPUVertexBuffer | GPUTransferDestinationBuffer, GFX_GPU_ONLY_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT);
	auto ib = NewGPUBuffer(idxByteSize, GFX_INDEX_BUFFER | GFX_TRANSFER_DESTINATION_BUFFER, GFX_GPU_ONLY_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT);
	theBuffer = NewGPUTransferCommandBuffer(GPUPersistentResource);
	theBuffer.CopyBuffer(vertByteSize, staging, vb, 0, 0);
	theBuffer.CopyBuffer(idxByteSize, staging, ib, vertByteSize, 0);
	QueueGPUCommandBuffer(theBuffer, NULL);
	ready = true;
	//auto fence = GfxCreateFence(false);
	//GfxSubmitCommandBuffers(1, &theBuffer, GFX_GRAPHICS_COMMAND_QUEUE, fence);
	return
	{
		.vertexBuffer = vb,
		.indexBuffer = ib,
	};
}

u32 QueueTextureUploadToGPU(u8 *pixels, s64 w, s64 h)
{
	// @TODO: Load texture directly into staging memory.
	auto staging = (void *){};
	auto nBytes = sizeof(u32) * w * h;
	auto stagingBuf = NewGPUBuffer(nBytes, GFX_TRANSFER_SOURCE_BUFFER, GFX_CPU_TO_GPU_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT, &stagingMemory);
	CopyMemory(pixels, stagingMemory, textureByteSize);
	auto image = CreateGPUImage(texturePixelWidth, texturePixelHeight, GFX_FORMAT_R8G8B8A8_UNORM, GFX_IMAGE_LAYOUT_UNDEFINED, GFX_IMAGE_USAGE_TRANSFER_DST | GFX_IMAGE_USAGE_SAMPLED, GFX_SAMPLE_COUNT_1, GFX_GPU_ONLY_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT);
	auto commandBuffer = CreateGPUCommandBuffer(GFX_TRANSFER_COMMAND_QUEUE, GPU_RESOURCE_LIFETIME_PERSISTENT);
	GfxTransitionImageLayout(commandBuffer, image, GFX_FORMAT_R8G8B8A8_UNORM, GFX_IMAGE_LAYOUT_UNDEFINED, GFX_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	GfxRecordCopyBufferToImageCommand(commandBuffer, stagingBuffer, image, texturePixelWidth, texturePixelHeight);
	GfxTransitionImageLayout(commandBuffer, image, GFX_FORMAT_R8G8B8A8_UNORM, GFX_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, GFX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	QueueGPUCommandBuffer(theBuffer, NULL);
	return 0;
}

#if DEVELOPMENT_BUILD
void LoadModel(ModelAsset *m, AssetID id)
{
	auto path = String{};
	switch (id)
	{
	case AssetIDSponza:
	{
		path = "Data/Models/Sponza/Sponza.gltf";
		m->name = "Sponza";
	} break;
	defualt:
	{
		Abort("Unknown asset id %d.\n", id);
	} break;
	}
	auto gltfModel = tinygltf::Model{};
	auto gltfLoader = tinygltf::TinyGLTF{};
	auto gltfErr = std::string{};
	auto gltfWarn = std::string{};
	auto gltfLoad = gltfLoader.LoadBinaryFromFile(&gltfModel, &gltfErr, &gltfWarn, &path[0]);
	if (!gltfWarn.empty())
	{
		LogInfo("Asset", "GLTF warning: %s.\n", gltfWarn.c_str());
	}
	if (!gltfErr.empty())
	{
		LogError("Asset", "GLTF error: %s.\n", gltfErr.c_str());
	}
	if (!gltfLoad)
	{
		LogError("Asset", "Failed to parse gltf asset file %k.\n", path);
	}
	Abort("", "");
#if 0
	auto jobParameter = (LoadModelJobParameter *)jobParameterPointer;

	auto modelFilepath = JoinFilepaths(String{"Data/Models"}, jobParameter->assetName, JoinStrings(jobParameter->assetName, ".bin"));

	auto gltfModel = tinygltf::Model{};
	auto gltfLoader = tinygltf::TinyGLTF{};
	auto gltfError = std::string{};
	auto gltfWarning = std::string{};

	auto gltfLoadResult = gltfLoader.LoadBinaryFromFile(&gltfModel, &gltfError, &gltfWarning, modelFilepath.data.elements);
	if (!gltfWarning.empty())
	{
		LogPrint(INFO_LOG, "GLTF warning: %s.\n", gltfWarning.c_str());
	}
	if (!gltfError.empty())
	{
		LogPrint(ERROR_LOG, "GLTF error: %s.\n", gltfError.c_str());
	}
	if (!gltfLoadResult)
	{
		LogPrint(ERROR_LOG, "Failed to parse gltf asset file %k.\n", modelFilepath);
	}

	auto mesh = AllocateStructMemory(MeshAsset);
	for (auto &gltfMesh : gltfModel.meshes)
	{
		auto gltfScene = gltfModel.scenes[gltfModel.defaultScene];
		for (auto &gltfNode : gltfScene.nodes)
		{
		}
	}
	Abort("TODO");
#endif

#if 0
	String modelName = GetFilepathFilename(modelDirectory);
	String fbxFilename = JoinStrings(modelName, ".fbx");
	String fbxFilepath = JoinFilepaths(modelDirectory, fbxFilename);
	auto assimpScene = aiImportFile(&fbxFilepath[0], aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_RemoveRedundantMaterials);
	if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
	{
		Abort("assimp error: %s", aiGetErrorString());
	}

	u32 submeshCount = assimpScene->mNumMeshes;
	//auto submeshCount = 1;

	ResizeArray(&mesh->submeshes, submeshCount);
	ResizeArray(&mesh->materials, submeshCount);
	for (auto i = 0; i < submeshCount; i++)
	{
		LoadTextureJobParameter *loadTextureJobParameters = (LoadTextureJobParameter *)malloc(5 * sizeof(LoadTextureJobParameter)); // @TODO
		loadTextureJobParameters[0] =
		LoadTextureJobParameter{
			.path = JoinFilepaths(modelDirectory, "_albedo.png"),
			.outputTextureID = &mesh->materials[i].albedo_map,
		};
		loadTextureJobParameters[1] =
		LoadTextureJobParameter{
			.path = JoinFilepaths(modelDirectory, "_normal.png"),
			.outputTextureID = &mesh->materials[i].normal_map,
		};
		loadTextureJobParameters[2] =
		LoadTextureJobParameter{
			.path = JoinFilepaths(modelDirectory, "_roughness.png"),
			.outputTextureID = &mesh->materials[i].roughness_map,
		};
		loadTextureJobParameters[3] =
		LoadTextureJobParameter{
			.path = JoinFilepaths(modelDirectory, "_metallic.png"),
			.outputTextureID = &mesh->materials[i].metallic_map,
		};
		loadTextureJobParameters[4] =
		LoadTextureJobParameter{
			.path = JoinFilepaths(modelDirectory, "_ambient_occlusion.png"),
			.outputTextureID = &mesh->materials[i].ambient_occlusion_map,
		};
		JobDeclaration loadTextureJobDeclarations[5]; // @TODO
		for (auto j = 0; j < CArrayCount(loadTextureJobDeclarations); j++)
		{
			loadTextureJobDeclarations[j] = CreateJob(LoadTexture, &loadTextureJobParameters[j]);
		}
		RunJobs(CArrayCount(loadTextureJobDeclarations), loadTextureJobDeclarations, NORMAL_PRIORITY_JOB, NULL);
	}

	auto totalVertexCount = 0;
	auto totalIndexCount = 0;
	for (auto i = 0; i < submeshCount; i++)
	{
		totalVertexCount += assimpScene->mMeshes[i]->mNumVertices;
		totalIndexCount += assimpScene->mMeshes[i]->mNumFaces * 3;
	}
	auto verticesSize = sizeof(Vertex1P1N) * totalVertexCount;
	auto indicesSize = sizeof(u32) * totalIndexCount;
	//ConsolePrint("totalVertexCount: %d, totalIndexCount: %d, verticesSize: %d, indicesSize: %d\n", totalVertexCount, totalIndexCount, verticesSize, indicesSize);

	mesh->vertex_count = totalVertexCount;
	mesh->index_count = totalIndexCount;
	*(jobParameter->outputMeshAssetAddress) = mesh;

	Vertex1P1N *vertex_buffer = (Vertex1P1N *)malloc(verticesSize); // @TODO
	u32 *index_buffer = (u32 *)malloc(indicesSize); // @TODO

	//void *vertexStagingMemory;
	//auto vertexStagingBuffer = CreateGPUStagingBuffer(verticesSize, (void **)&vertexStagingMemory);

	//void *indexStagingMemory;
	//auto indexStagingBuffer = CreateGPUStagingBuffer(indicesSize, (void **)&indexStagingMemory);

	void *stagingMemory;
	auto stagingBuffer = CreateGPUBuffer(verticesSize + indicesSize, GFX_TRANSFER_SOURCE_BUFFER, GFX_CPU_TO_GPU_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT, &stagingMemory);

	auto meshVertexOffset = 0;
	auto meshIndexOffset = 0;
	auto max_x_vertex = 0, max_y_vertex = 0, max_z_vertex = 0;
	auto min_x_vertex = 0, min_y_vertex = 0, min_z_vertex = 0;
	for (auto i = 0; i < submeshCount; i++)
	{
		struct aiMesh *assimp_mesh = assimpScene->mMeshes[i];
		assert(assimp_mesh->mVertices && assimp_mesh->mNormals && (assimp_mesh->mFaces && assimp_mesh->mNumFaces > 0));

		//u32 vertices_size = sizeof(Vertex) * assimp_mesh->mNumVertices;
		//u32 vertices_size = sizeof(Vertex1P1N) * assimp_mesh->mNumVertices;
		//u32 indices_size = sizeof(u32) * 3 * assimp_mesh->mNumFaces;
		//Vertex *vertex_buffer = malloc(vertices_size); // @TODO

		for (auto j = 0; j < assimp_mesh->mNumVertices; j++)
		{
			auto v = &vertex_buffer[meshVertexOffset + j];
			v->position =
			{
				assimp_mesh->mVertices[j].x,
				-assimp_mesh->mVertices[j].z,
				assimp_mesh->mVertices[j].y,
			};
			v->normal =
			{
				assimp_mesh->mNormals[j].x,
				assimp_mesh->mNormals[j].y,
				assimp_mesh->mNormals[j].z,
			};
			//ConsolePrint("%d: %f %f %f, %f %f %f\n", j, v->position.x, v->position.y, v->position.z, v->normal.x, v->normal.y, v->normal.z);

#if 0
			if (v->position.x > assimp_mesh->mVertices[max_x_vertex].x) {
				max_x_vertex = j;
			}
			if (v->position.y > assimp_mesh->mVertices[max_y_vertex].y) {
				max_y_vertex = j;
			}
			if (v->position.z > assimp_mesh->mVertices[max_z_vertex].z) {
				max_z_vertex = j;
			}
			if (v->position.x < assimp_mesh->mVertices[min_x_vertex].x) {
				min_x_vertex = j;
			}
			if (v->position.y < assimp_mesh->mVertices[min_y_vertex].y) {
				min_y_vertex = j;
			}
			if (v->position.z < assimp_mesh->mVertices[min_z_vertex].z) {
				min_z_vertex = j;
			}
			v->normal = (V3){
				.x = assimp_mesh->mNormals[j].x,
				.y = assimp_mesh->mNormals[j].y,
				.z = assimp_mesh->mNormals[j].z,
			};
			v->color = (V3){
				1.0f,
				0.0f,
				0.0f,
			};
			v->tangent = (V3){
				.x = assimp_mesh->mTangents[j].x,
				.y = assimp_mesh->mTangents[j].y,
				.z = assimp_mesh->mTangents[j].z,
			};
			if (assimp_mesh->mTextureCoords[0]) {
				v->uv.x = assimp_mesh->mTextureCoords[0][j].x;
				v->uv.y = assimp_mesh->mTextureCoords[0][j].y;
			} else {
				v->uv.x = -1;
				v->uv.y = -1;
			}
#endif
		}

		// @TODO: Tighter bounding sphere calculation.
		V3 bounding_box_center = {
			(assimp_mesh->mVertices[max_x_vertex].x + assimp_mesh->mVertices[min_x_vertex].x) / 2.0f,
			(assimp_mesh->mVertices[max_y_vertex].y + assimp_mesh->mVertices[min_y_vertex].y) / 2.0f,
			(assimp_mesh->mVertices[max_z_vertex].z + assimp_mesh->mVertices[min_z_vertex].z) / 2.0f,
		};
		V3 max = {
			assimp_mesh->mVertices[max_x_vertex].x,
			assimp_mesh->mVertices[max_y_vertex].y,
			assimp_mesh->mVertices[max_z_vertex].z,
		};
		V3 min = {
			assimp_mesh->mVertices[min_x_vertex].x,
			assimp_mesh->mVertices[min_y_vertex].y,
			assimp_mesh->mVertices[min_z_vertex].z,
		};
		V3 bounding_box_max_corner = {
			assimp_mesh->mVertices[max_x_vertex].x,
			assimp_mesh->mVertices[max_y_vertex].y,
			assimp_mesh->mVertices[max_z_vertex].z,
		};
		mesh->boundingSphere = (BoundingSphere){
			.center = bounding_box_center,
			.radius = Length(bounding_box_max_corner - bounding_box_center),
		};

		for (auto j = 0; j < assimp_mesh->mNumFaces; j++)
		{
			Assert(assimp_mesh->mFaces[j].mNumIndices == 3);
			index_buffer[meshIndexOffset + (3 * j) + 0] = meshVertexOffset + assimp_mesh->mFaces[j].mIndices[0];
			index_buffer[meshIndexOffset + (3 * j) + 1] = meshVertexOffset + assimp_mesh->mFaces[j].mIndices[1];
			index_buffer[meshIndexOffset + (3 * j) + 2] = meshVertexOffset + assimp_mesh->mFaces[j].mIndices[2];
		}

		mesh->submeshes[i].indexCount = 3 * assimp_mesh->mNumFaces;
		mesh->submeshes[i].firstIndex = meshIndexOffset;
		mesh->submeshes[i].vertexOffset = 0;

		meshVertexOffset += assimp_mesh->mNumVertices;
		meshIndexOffset += 3 * assimp_mesh->mNumFaces;
#if 0
			struct aiString diffuse_path;
			if (aiGetMaterialTexture(assimp_material, aiTextureType_DIFFUSE, 0, &diffuse_path, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS) {
				//material->shader = TEXTURED_STATIC_SHADER;
				const char *texture_path = join_paths(model_directory, diffuse_path.data, thread_local_arena);
				material->diffuse_map = load_texture(texture_path, assets);
			} else {
				//material->shader = UNTEXTURED_STATIC_SHADER;
				material->diffuse_map = INVALID_ID;
			}
			struct aiString specular_path;
			if (aiGetMaterialTexture(assimp_material, aiTextureType_SPECULAR, 0, &specular_path, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS) {
				const char *texture_path = join_paths(model_directory, specular_path.data, thread_local_arena);
				//material->specular_map = load_texture(texture_path, assets);
			} else {
				material->specular_map = INVALID_ID;
			}
			struct aiString normal_path;
			//if (aiGetMaterialTexture(assimp_material, aiTextureType_NORMALS, 0, &normal_path, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS) {
				//assert(0);
				const char *texture_path = join_paths(model_directory, "Normal.png", thread_local_arena);
				material->normal_map = load_texture(texture_path, assets);
			//} else {
				//material->normal_map = INVALID_ID;
			//}
			struct aiColor4D diffuse_color, specular_color;
			if (aiGetMaterialColor(assimp_material, AI_MATKEY_COLOR_DIFFUSE, &diffuse_color) == aiReturn_SUCCESS) {
				material->diffuse_color = (V3){diffuse_color.r, diffuse_color.g, diffuse_color.b};
			} else {
				material->diffuse_color = DEFAULT_DIFFUSE_COLOR;
			}
			if (aiGetMaterialColor(assimp_material, AI_MATKEY_COLOR_DIFFUSE, &diffuse_color) == aiReturn_SUCCESS) {
				material->diffuse_color = (V3){diffuse_color.r, diffuse_color.g, diffuse_color.b};
			} else {
				material->diffuse_color = DEFAULT_SPECULAR_COLOR;
			}
#endif
	}

	CopyMemory(vertex_buffer, stagingMemory, verticesSize);
	CopyMemory(index_buffer, (char *)stagingMemory + verticesSize, indicesSize);
	mesh->gpuGeometry = QueueIndexedGeometryUploadToGPU(verticesSize, indicesSize, stagingBuffer);
	//mesh->gpu_mesh = Upload_Indexed_Geometry_To_GPU(mesh->vertex_count, sizeof(Vertex), mesh->vertices, mesh->index_count, mesh->indices);

#endif

	mesh->loadStatus = ASSET_LOADED;
}
#endif

void LoadTexture(String path)
{
	auto texW = s32{}, texH = s32{}, texChans = s32{};
	auto pixels = stbi_load(&path[0], &texW, &texH, &texChans, STBI_rgb_alpha);
	if (!pixels)
	{
		LogError("Asset", "Failed to load texture %k.\n", path);
		return;
	}
	Defer(FreeMemory(pixels));
	QueueTextureUploadToGPU(pixels, texW, texH);
}

void *LockAsset(AssetID id)
{
	auto c = &assets.containers[id];
	AcquireSpinLock(c->lock);
	Defer(ReleaseSpinLock(assets.lock));
	if (!c->resident)
	{
		LogError("Asset", "Tried to lock asset %k, but asset is not resident.\n", c->name);
		return NULL;
	}
	if (c->loadCount == 0)
	{
		LogError("Asset", "Tried to lock asset %k, but asset is scheduled for unload.\n", c->name);
		return NULL;
	}
	if (c->loading)
	{
		return NULL;
	}
	Assert(!c->loadCount > 0 && !c->loading);
	c->lockCount += 1;
	return c->asset;
}

void DoAssetUnload(AssetContainer *c)
{
	switch (c->type)
	{
	case AssetTypeModel:
	{
		UnloadModelAsset((ModelAsset *)c->asset);
	} break;
	default: {
		Abort("Unknown asset type %d.\n", c->type);
	} break;
	}
	AcquireSpinLock(assets.allocatorLock);
	FreeSlotMemory(assets.slots[c->type]);
	ReleaseSpinLock(assets.allocatorLock);
}

void UnlockAsset(AssetID id)
{
	auto unload = false;
	auto c = &assets.containers[id];
	{
		AcquireSpinLock(c->lock);
		if (!c->resident)
		{
			LogError("Tried to unlock asset %k, but asset is not resident.\n", c->name);
			return;
		}
		c->lockCount -= 1;
		if (c->lockCount == 0 && c->loadCount == 0)
		{
			unload = true;
			c->resident = false;
		}
		ReleaseSpinLock(assets.lock);
	}
	if (unload)
	{
		DoAssetUnload(c);
	}
}

void LoadAsset(void *jobParams)
{
	auto params = (LoadAssetJobParameters *)jobParams;
	auto c = &assets.containers[params->id];
	AcquireSpinLock(c->lock);
	c->loadCount += 1;
	// Check if the asset is already loaded.
	if (c->resident)
	{
		// The asset is either already loaded or is in the process of loading.
		if (params->type != c->type || params->path != c->path)
		{
			LogError("Tried to load asset %d:%d:%k, but a different asset with the same id %d:%d:%k is already loaded.\n", params->id, params->type, params->path, c->id, c->type, c->path);
		}
		if (c->scheduledForUnload)
		{
			c->scheduledForUnload = false;
		}
		return;
	}
	c->resident = true;
	c->loading = true;
	// The asset is not loaded, so we need to actually load the asset from file.
	// Insert a reference to the asset into the hash table so the asset system knows the asset is loaded/loading.
	ReleaseSpinLock(c->lock);
	// We released the spinlock while the asset is in the loading state, so no one can actually lock or use the asset yet.
	// Now, we can do the slow load from disk without holding the asset lock.
	auto asset = (void *){};
	switch (params->type)
	{
	case AssetTypeModel:
	{
		AcquireSpinLock(assets.allocatorLock);
		asset = (ModelAsset *)AllocateSlotMemory(&assets.slots[params->type], sizeof(ModelAsset)),
		ReleaseSpinLock(assets.allocatorLock);
		LoadModelAsset(asset, params->id);
		#if DEBUG_BUILD
			c->name = asset->name;
		#endif
	} break;
	default:
	{
		Abort("Unknown asset type %d.\n", params->type);
	} break;
	}
	AcquireSpinLock(c->lock);
	c->asset = asset;
	c->loading = false;
	ReleaseSpinLock(c->lock);
}

void UnloadAsset(AssetID id)
{
	auto c = &assets.containers[id];
	AcquireSpinLock(c->lock);
	if (!c->resident)
	{
		LogError("Tried to unload asset %k, but asset is not resident.\n", assets.holders[id].name);
		return;
	}
	c->loadCount -= 1;
	auto unload = false;
	if (c->lockCount == 0 && c->loadCount == 0)
	{
		unload = true;
		c->resident = false;
	}
	ReleaseSpinLock(c->lock);
	if (unload)
	{
		DoUnloadAsset(c);
	}
}

String AssetIDToString(AssetID id)
{
	switch (id)
	{
	case AssetIDSponza:
	{
		return "AssetIDSponza";
	} break;
	}
	return "NoAssetIDToStringRecord";
}
#endif
