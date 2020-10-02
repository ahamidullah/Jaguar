#include "Model.h"
#include "GLTF.h"
#include "Basic/File.h"
#include "Basic/Filepath.h"
#include "Basic/HashTable.h"
#include "Basic/Hash.h"
#include "Basic/Parser.h"

#ifdef DevelopmentBuild
	const auto ModelDirectory = NewString("Data/Model");

	auto modelFilepaths = NewHashTableIn<String, String>(GlobalAllocator(), 0, HashString);
#endif

void InitializeModelAssets()
{
	if (DevelopmentBuild)
	{
		auto itr = DirectoryIteration{};
		while (itr.Iterate(ModelDirectory))
		{
			if (!itr.isDirectory)
			{
				LogVerbose("Model", "File %k is not a directory, skipping.", itr.filename);
				continue;
			}
			auto name = itr.filename;
			auto path = JoinFilepaths(ModelDirectory, itr.filename, "glTF", JoinStrings(name, ".gltf"));
			if (!FileExists(path))
			{
				LogVerbose("Model", "%k does not exist in directory %k, skipping.", JoinStrings(name, ".gltf"), JoinFilepaths(ModelDirectory, itr.filename, "glTF"));
				continue;
			}
			modelFilepaths.Insert(name.CopyIn(GlobalAllocator()), path.CopyIn(GlobalAllocator()));
			LogVerbose("Model", "Registered %k to filepath %k.", name, path);
		}
	}
}

void LoadModelAssetFromFile(String name)
{
	auto path = modelFilepaths.Lookup(name, "");
	if (path == "")
	{
		LogError("Model", "Failed to find a registered file path for %k, skipping load.", name);
		return;
	}
	auto err = false;
	auto gltf = GLTFParseFile(path, &err);
	if (err)
	{
		LogError("Model", "Failed to parse glTF file for %k, skipping load.", path);
		return;
	}
}

void LoadModelAsset(String name)
{
	if (DevelopmentBuild)
	{
		LoadModelAssetFromFile(name);
	}
	else
	{
		Abort("Model", "@TODO: Load model assets in release mode.");
	}
}

#if 0
void LoadModelAsset(void *jobParameterPointer)
{
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
#endif
