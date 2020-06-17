#include "Asset.h"
#include "GPU.h"
#include "PCH.h"
#include "Mesh.h"
#include "Job.h"

#include "Code/Basic/Log.h"
#include "Code/Basic/Filesystem.h"
#include "Code/Basic/HashTable.h"
#include "Code/Basic/Hash.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"
#undef TINYGLTF_IMPLEMENTATION

// @TODO
#define ASSET_COUNT 100

enum AssetType
{
	MODEL_ASSET,

	ASSET_TYPE_COUNT
};

// Subassets can be referenced by assets (including other Subassets), but are not usable outside of the asset system.
template <typename T>
struct Subassets
{
	T data;
	s64 referenceCount;
}

struct AssetReference
{
	void *asset;
	AssetType type;
	bool loading;
	bool scheduledForUnload;
	s64 lockCount;
	String filepath;
};

struct AssetGlobals
{
	SpinLock lock;
	HashTable<String, AssetReference> lookup;
	SlotAllocator assetAllocators[ASSET_TYPE_COUNT];
	HeapAllocator heap;
} assetGlobals;

void InitializeAssets(void *)
{
	assetGlobals.heapAllocator = CreateHeapAllocator(KilobytesToBytes(64), 2, systemMemoryAllocator, systemMemoryAllocator);
	for (auto i = 0; i < ASSET_TYPE_COUNT; i++)
	{
		switch (i)
		{
		case MODEL_ASSET:
		{
			assetGlobals.slotAllocators[i] = CreateSlotAllocator(KilobytesToBytes(64), 0, assetGlobals.heapAllocator, assetGlobals.heapAllocator);
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

GPUIndexedGeometry QueueIndexedGeometryUploadToGPU(s64 verticesByteSize, s64 indicesByteSize, GfxBuffer vertexStagingBuffer, AssetLoadStatus *loadStatus)
{
	auto vertexBuffer = CreateGPUBuffer(verticesByteSize, GFX_VERTEX_BUFFER | GFX_TRANSFER_DESTINATION_BUFFER, GFX_GPU_ONLY_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT);
	auto indexBuffer = CreateGPUBuffer(indicesByteSize, GFX_INDEX_BUFFER | GFX_TRANSFER_DESTINATION_BUFFER, GFX_GPU_ONLY_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT);
	//auto commandBuffer = CreateUploadCommandBuffer();
	theBuffer = CreateGPUCommandBuffer(GFX_TRANSFER_COMMAND_QUEUE, GPU_RESOURCE_LIFETIME_PERSISTENT);
	GfxRecordCopyBufferCommand(theBuffer, verticesByteSize, vertexStagingBuffer, vertexBuffer, 0, 0);
	GfxRecordCopyBufferCommand(theBuffer, indicesByteSize, vertexStagingBuffer, indexBuffer, verticesByteSize, 0);
	QueueGPUCommandBuffer(theBuffer, NULL);
	ready = true;
	//auto fence = GfxCreateFence(false);
	//GfxSubmitCommandBuffers(1, &theBuffer, GFX_GRAPHICS_COMMAND_QUEUE, fence);
	return
	{
		.vertexBuffer = vertexBuffer,
		.indexBuffer = indexBuffer,
	};
}

u32 QueueTextureUploadToGPU(u8 *pixels, s64 texturePixelWidth, s64 texturePixelHeight, AssetLoadStatus *loadStatus)
{
	// @TODO: Load texture directly into staging memory.
	void *stagingMemory;
	auto textureByteSize = sizeof(u32) * texturePixelWidth * texturePixelHeight;
	auto stagingBuffer = CreateGPUBuffer(textureByteSize, GFX_TRANSFER_SOURCE_BUFFER, GFX_CPU_TO_GPU_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT, &stagingMemory);
	CopyMemory(pixels, stagingMemory, textureByteSize);
	auto image = CreateGPUImage(texturePixelWidth, texturePixelHeight, GFX_FORMAT_R8G8B8A8_UNORM, GFX_IMAGE_LAYOUT_UNDEFINED, GFX_IMAGE_USAGE_TRANSFER_DST | GFX_IMAGE_USAGE_SAMPLED, GFX_SAMPLE_COUNT_1, GFX_GPU_ONLY_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT);
	auto commandBuffer = CreateGPUCommandBuffer(GFX_TRANSFER_COMMAND_QUEUE, GPU_RESOURCE_LIFETIME_PERSISTENT);
	GfxTransitionImageLayout(commandBuffer, image, GFX_FORMAT_R8G8B8A8_UNORM, GFX_IMAGE_LAYOUT_UNDEFINED, GFX_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	GfxRecordCopyBufferToImageCommand(commandBuffer, stagingBuffer, image, texturePixelWidth, texturePixelHeight);
	GfxTransitionImageLayout(commandBuffer, image, GFX_FORMAT_R8G8B8A8_UNORM, GFX_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, GFX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	QueueGPUCommandBuffer(theBuffer, NULL);
	return 0;
}

void LoadModelAsset(String name, String filepath, Model *model)
{
	//auto modelFilepath = JoinFilepaths(String{"Data/Models"}, jobParameter->assetName, JoinStrings(jobParameter->assetName, ".bin"));

	auto gltfModel = tinygltf::Model{};
	auto gltfLoader = tinygltf::TinyGLTF{};
	auto gltfError = std::string{};
	auto gltfWarning = std::string{};

	auto gltfLoadResult = gltfLoader.LoadBinaryFromFile(&gltfModel, &gltfError, &gltfWarning, filepath.data.elements);
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
		LogPrint(ERROR_LOG, "Failed to parse gltf asset file %k.\n", filepath);
	}
	Abort("");
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
	mesh->gpuGeometry = QueueIndexedGeometryUploadToGPU(verticesSize, indicesSize, stagingBuffer, &mesh->loadStatus);
	//mesh->gpu_mesh = Upload_Indexed_Geometry_To_GPU(mesh->vertex_count, sizeof(Vertex), mesh->vertices, mesh->index_count, mesh->indices);

#endif

	mesh->loadStatus = ASSET_LOADED;
}

struct LoadTextureJobParameter
{
	String path;
	AssetLoadStatus *loadStatus;
	u32 *outputTextureID;
};

void LoadTexture(void *jobParameterPointer)
{
	LoadTextureJobParameter *jobParameter = (LoadTextureJobParameter *)jobParameterPointer;
	s32 texturePixelWidth, texturePixelHeight, textureChannels;
	u8 *pixels = stbi_load(&jobParameter->path[0], &texturePixelWidth, &texturePixelHeight, &textureChannels, STBI_rgb_alpha);
	if (!pixels)
	{
		LogPrint(ERROR_LOG, "failed to load texture %s\n", &jobParameter->path[0]);
		return;
	}
	Defer(free(pixels));
	*jobParameter->outputTextureID = QueueTextureUploadToGPU(pixels, texturePixelWidth, texturePixelHeight, jobParameter->loadStatus);
}


void *LockAsset(String assetName)
{
	AcquireSpinLock(assetGlobals.lock);
	Defer(ReleaseSpinLock(assetGlobals.lock));

	auto assetReference = LookupInHashTable(&assetGlobals.lookup, assetName);
	if (!assetReference)
	{
		LogPrint(ERROR_LOG, "Tried to lock asset %k, but asset does not exist.\n", assetName);
		return NULL;
	}
	if (assetReference->scheduledForUnload)
	{
		LogPrint(ERROR_LOG, "Tried to lock asset %k, but asset is scheduled for unload.\n", assetName);
		return NULL;
	}
	if (assetReference->loading)
	{
		return NULL;
	}
	Assert(!assetReference->scheduledForUnload && !assetReference->loading);
	assetReference->lockCount += 1;
	return assetReference->asset;
}

void UnlockAsset(String assetName)
{
	auto unloadAsset = (void *){};
	auto unloadAssetType = AssetType{};
	{
		AcquireSpinLock(assetGlobals.lock);

		auto assetReference = LookupInHashTable(&assetGlobals.lookup, assetName);
		if (!assetReference)
		{
			LogPrint(ERROR_LOG, "Tried to unlock asset %k, but asset does not exist.\n", assetName);
			return;
		}
		assetReference->lockCount -= 1;
		if (assetReference->lockCount == 0 && assetReference->scheduledForUnload)
		{
			unloadAsset = assetReference->asset;
			unloadAssetType = assetReference->assetType;
			RemoveFromHashTable(&assetGlobals.lookup, assetName);
			// Once we unlock the spinlock, this asset will be capable of loading again.
			// This means we can have the same asset simultanously loading and unloading.
		}

		ReleaseSpinLock(assetGlobals.lock);
	}
	if (unloadAsset)
	{
		switch (unloadAssetType)
		{
		case MODEL_ASSET:
		{
			UnloadModelAsset((ModelAsset *)unloadAsset->data);
		} break;
		default: {
			Abort("Unknown asset type %d.\n", unloadAssetType);
		} break;
		}
		FreeSlotMemory(unloadAsset);
	}
}

struct LoadAssetJobParameters
{
	String assetName;
	AssetType assetType;
	String assetFilepath;
};

void LoadAsset(void *jobParameter)
{
	auto parameters = (LoadAssetJobParameters *)jobParameter;

	AcquireSpinLock(assetGlobals.lock);

	if (auto assetReference = LookupInHashTable(&assetGlobals.lookup, parameters->assetName); assetReference)
	{
		// The asset is either already loaded or is in the process of loading.
		if (parameters->assetType != assetReference->type || parameters->assetFilepath != assetReference->filepath)
		{
			LogPrint(ERROR_LOG, "Tried to load asset %k:%d:%k, but a different asset with the same name %k:%d:%k is already loaded.\n", parameters->assetName, parameters->assetType, parameters->assetFilepath, assetReference->name, assetReference->type, assetReference->filepath);
		}
		if (assetReference->scheduledForUnload)
		{
			assetReference->scheduledForUnload = false;
		}
		return;
	}

	// The asset is not loaded, so we need to actually load the asset from file.
	InsertIntoHashTable(
		&assetGlobals.lookup,
		parameters->assetName,
		AssetReference
		{
			.type = parameters->assetType,
			.loading = true,
			.filepath = parameters->assetFilepath,
		});

	ReleaseSpinLock(assetGlobals.lock);

	auto asset = (void *){};
	switch (parameters->assetType)
	{
	case MODEL_ASSET:
	{
		asset = (ModelAsset *)AllocateSlotMemory(&assetGlobals.assetAllocators[parameters->assetType], sizeof(ModelAsset)),
		LoadModelAsset(asset, parameters->assetName, parameters->assetFilepath);
	} break;
	default:
	{
		Abort("Unknown asset type %d.\n", parameters->assetType);
	} break;
	}

	AcquireSpinLock(assetGlobals.lock);

	// Pointers aren't stable once we release the lock, so we have to lookup the asset again.
	auto newAssetReference = LookupInHashTable(&assetGlobals.lookup, parameters->assetName);
	newAssetReference->asset = asset;
	newAssetReference->loading = false;

	ReleaseSpinLock(assetGlobals.lock);
}

void UnloadAssetWhenFullyUnlocked(String assetName)
{
	// @TODO
	// Mark the asset for unload then actually do it when its lockCount hits zero.
	// Be careful not to unload assets which are still loading.
	// Should be fine if everyone locks the table correctly.
	auto asset = LookupInHashTable(&assetGlobals.lookup, parameters->assetName);
	if (!asset)
	{
		LogPrint(ERROR_LOG, "Tried to unload asset %k, but asset does not exist.\n", assetName);
		return;
	}
	asset->scheduledForUnload = true;
}






void GetLockedAsset(void *jobParameters)
{
	auto parameters = (GetLockedAssetJobParameters *)jobParameters;
	return LoadLockedAssetActual();
	AcquireSpinLock(assetGlobals.lock);
	auto *asset = LoadAssetActual(parameters->assetType, parameters->assetName);
	asset->lockCount += 1;
	ReleaseSpinLock(assetGlobals.lock);

#if 0
	AcquireSpinLock(assetGlobals.lock);
	auto needsToBeLoaded = false;
	auto asset = LookupInHashTable(&assetGlobals.lookupTable, AssetLookupKey{parameters->assetType, parameters->assetName});
	if (!asset)
	{
	#if 0
		switch (parameters->assetType)
		{
		case MODEL_ASSET:
		{
			ResizeArray(&assetGlobals.models, assetGlobals.models.count + 1);
			asset = &assetGlobals.models[assetGlobals.models.count - 1];
		} break;
		case MESH_ASSET:
		{
			ResizeArray(&assetGlobals.models, assetGlobals.meshes.count + 1);
			asset = &assetGlobals.models[assetGlobals.meshes.count - 1];
		} break;
		case TEXTURE_ASSET:
		{
			// @TODO
		} break;
		default:
		{
			Abort("Unknown asset type %d.\n", parameters->assetType);
		} break;
		}
		InsertIntoHashTable(&assetGlobals.lookupTable, parameters->assetName, asset);
	#endif
		needsToBeLoaded = true;
	}
	else
	{
		asset->lockCount += 1;
	}
	ReleaseSpinLock(assetGlobals.lock);

	if (needsToBeLoaded)
	{
		DoLoadAsset(parameters->assetName);
		asset = LoadAsset();
		switch (parameters->assetType)
		{
		case MODEL_ASSET:
		{
			LoadModelAsset();
		} break;
		case MESH_ASSET:
		{
			LoadMeshAsset();
		} break;
		case TEXTURE_ASSET:
		{
		} break;
		default:
		{
			Abort("Unknown asset type %d.\n", parameters->assetType);
		} break;
		}
	}
#endif
}

void UnlockAsset(String name)
{
	AcquireSpinLock(assetGlobals.lock);
	Defer(ReleaseSpinLock(assetGlobals.lock));
	auto asset = LookupInHashTable(&assetGlobals.catalog);
	Assert(asset);
	asset->lockCount -= 1;
}

template <typename T>
void GetAsset(void *jobParameterPointer)
{
	auto Load() = [&name]()
	{
		// @TODO: Call load model asset. No outputMeshAssetAddress.
		LoadModelJobParameter *jobParameter = (LoadModelJobParameter *)malloc(sizeof(LoadModelJobParameter)); // @TODO
		jobParameter->assetName = name;
		jobParameter->outputMeshAssetAddress = &addr;
		JobDeclaration jobDeclaration = CreateJob(LoadModel, jobParameter);
		JobCounter jobCounter;
		RunJobs(1, &jobDeclaration, NORMAL_PRIORITY_JOB, &jobCounter);
		WaitForJobCounter(&jobCounter); // @TODO: Get rid of this wait!
	}

	AcquireSpinLock(assetGlobals.catalogLock);
	auto asset = LookupInHashTable(&assetGlobals.catalog, name);
	if (asset && asset->loadStatus == ASSET_LOADED)
	{
		auto IsCorrectAssetLoaded = []()
		{
		}
		AtomicCompareAndSwap(&modelAsset->loadStatus, ASSET_LOADED, ASSET_LOCKED_FOR_FRAME_DURATION);
		if (modelAsset->loadStatus != ASSET_LOADED || modelAsset->name != name)
		{
			Load();
			return NULL;
		}
		auto meshAsset = LookupInHashTable(&assetGlobals.catalog, modelAsset->meshAssetName);
		return (ModelAsset *)modelAsset;
	}
	else
	if (!modelAssetPointer)
	{
		Load();
	}
	return NULL;
}

bool done = false;
GfxFence theFence;

#if 0
void FinalizeAssetUploadsToGPU()
{
	if (ready && !done && theBuffer)
	{
		theFence = GfxCreateFence(false);
		GfxSubmitCommandBuffers(1, &theBuffer, GFX_GRAPHICS_COMMAND_QUEUE, theFence);
		//VK_CHECK(vkQueueWaitIdle(vulkanGlobals.graphicsQueue)); // @TODO: Use a fence?
		done = true;
	}
	if (done)
	{
		if (GfxWasFenceSignalled(theFence))
		{
			*assetGlobals.uploadCommandBuffers.writeBuffer->loadStatuses[0] = ASSET_LOADED;
		}
	}
	return;

	// Submit queued asset upload commands.
	SwitchAssetUploadDoubleBuffer(&assetGlobals.uploadCommandBuffers);
	if (assetGlobals.uploadCommandBuffers.readBufferElementCount > 0)
	{
		ConsolePrint("READING FROM: %x\n", assetGlobals.uploadCommandBuffers.readBuffer->commandBuffers);
		ConsolePrint("c: %u\n", assetGlobals.uploadCommandBuffers.readBufferElementCount);
		GfxSubmitCommandBuffers(assetGlobals.uploadCommandBuffers.readBufferElementCount, assetGlobals.uploadCommandBuffers.readBuffer->commandBuffers, GFX_GRAPHICS_COMMAND_QUEUE, assetGlobals.uploadFences[assetGlobals.uploadFenceCount]);
		assetGlobals.pendingLoadStatusCounts[assetGlobals.uploadFenceCount] = assetGlobals.uploadCommandBuffers.readBufferElementCount;
		CopyMemory(assetGlobals.uploadCommandBuffers.readBuffer->loadStatuses, &assetGlobals.pendingLoadStatuses[assetGlobals.uploadFenceCount], assetGlobals.uploadCommandBuffers.readBufferElementCount * sizeof(AssetLoadStatus *));
		assetGlobals.uploadFenceCount += 1;
		// @TODO: Render_API_Free_Command_Buffers(assetGlobals.gpu_upload_command_pool, assetGlobals.uploadCommandBuffers.readBufferElementCount, assetGlobals.uploadCommandBuffers.read_buffer->command_buffers);
	}

	// Check whether submitted asset upload commands have finished.
	for (auto i = 0; i < assetGlobals.uploadFenceCount; i++)
	{
		if (!GfxWasFenceSignalled(assetGlobals.uploadFences[i]))
		{
			continue;
		}
		GfxResetFences(1, &assetGlobals.uploadFences[i]);
		for (auto j = 0; j < assetGlobals.pendingLoadStatusCounts[i]; j++)
		{
			*assetGlobals.pendingLoadStatuses[i][j] = ASSET_LOADED;
		}
		// Remove the fence.
		assetGlobals.uploadFenceCount -= 1;
		if (i != assetGlobals.uploadFenceCount)
		{
			GfxFence temporary = assetGlobals.uploadFences[i];
			assetGlobals.uploadFences[i] = assetGlobals.uploadFences[assetGlobals.uploadFenceCount];
			assetGlobals.uploadFences[assetGlobals.uploadFenceCount] = temporary;
			i -= 1;
		}
	}
}
#endif
