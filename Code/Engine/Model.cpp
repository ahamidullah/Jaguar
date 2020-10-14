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

//auto meshes = StaticArray<MeshAsset, 10000>{};
//auto meshes = StaticArray<GPUMesh, 10000>{};
//auto meshes = NewGPUMeshes(10000);
//const auto MeshCount = 10000;
const auto MeshCount = 1;
auto meshes = NewArray<GPUMesh>(MeshCount);

ModelAsset LoadModelAssetFromFile(String name)
{
	auto gltfPath = modelFilepaths.Lookup(name, "");
	if (gltfPath == "")
	{
		LogError("Model", "Failed to find a registered file path for %k, skipping load.", name);
		return {};
	}
	auto err = false;
	auto gltf = GLTFParseFile(gltfPath, &err);
	if (err)
	{
		LogError("Model", "Failed to parse glTF file for %k, skipping load.", gltfPath);
		return {};
	}
	auto buffers = Array<Array<u8>>{};
	for (auto b : gltf.buffers)
	{
		auto p = JoinFilepaths(FilepathDirectory(gltfPath), b.uri);
		auto f = ReadEntireFile(p, &err);
		if (err)
		{
			LogError("Model", "Failed to read glTF URI file %k.", p);
			return {};
		}
		buffers.Append(f);
	}
	auto cb = NewGPUFrameTransferCommandBuffer();
	auto vertexCount = 0;
	for (auto m : gltf.meshes)
	{
		for (auto i = 0; i < MeshCount; i += 1)
		{
		for (auto p : m.primitives)
		{
			meshes[i] = NewGPUMesh(sizeof(Vertex1P1N) * 24, sizeof(u16) * 36);
			auto m = &meshes[i];
			// Indices.
			{
				auto acc = &gltf.accessors[p.indices];
				auto indicesSize = acc->count * acc->type * GLTFComponentTypeToSize(acc->componentType);
				switch (GLTFComponentTypeToSize(acc->componentType))
				{
				case 2:
				{
					//meshes[i].indexType = GPUIndexTypeUint16;
					//it = GPUIndexTypeUint16;
				} break;
				case 4:
				{
					//meshes[i].indexType = GPUIndexTypeUint32;
					//it = GPUIndexTypeUint32;
				} break;
				default:
				{
					Abort("Model", "Incompatible glTF mesh index size %d.", GLTFComponentTypeToSize(acc->componentType));
				}
				}
				//auto im = m->MapIndexBuffer();
				//ic = acc->count;
				//ib = buffers[bv->buffer].elements + bv->byteOffset + acc->byteOffset; // @TODO: mmap?
				//meshes[i].indexCount = acc->count;
				//meshes[i].indexBuffer = NewGPUIndexBuffer(indicesSize);
				//meshes[i].firstIndex = i * 36;// + padding;
				auto s = NewGPUFrameStagingBufferX(m->IndexBuffer(), indicesSize, 0);
				auto bv = &gltf.bufferViews[acc->bufferView];
				auto b = buffers[bv->buffer].elements + bv->byteOffset + acc->byteOffset;
				CopyArray(NewArrayView(b, indicesSize), NewArrayView((u8 *)s.Map(), indicesSize));
				s.FlushIn(cb);
			}
			// Vertices.
			{
				auto verticesSize = 0;
				auto vertexCount = 0;
				auto posAccessIndex = -1;
				auto normAccessIndex = -1;
				for (auto a : p.attributes)
				{
					if (a.type == GLTFPositionType)
					{
						auto acc = &gltf.accessors[a.index];
						verticesSize += acc->count * acc->type * GLTFComponentTypeToSize(acc->componentType);
						posAccessIndex = a.index;
						vertexCount = acc->count;
					}
					else if (a.type == GLTFNormalType)
					{
						auto acc = &gltf.accessors[a.index];
						verticesSize += acc->count * acc->type * GLTFComponentTypeToSize(acc->componentType);
						normAccessIndex = a.index;
					}
				}
				// Copy vertex data to GPU buffer, making sure it is interleaved.
				//auto iv = m->MapVertexBuffer();
				//meshes[i].vertexBuffer = NewGPUVertexBuffer(verticesSize);
				//meshes[i].vertexOffset = i * 24;// + padding;
				auto s = NewGPUFrameStagingBufferX(m->VertexBuffer(), verticesSize, 0);
				auto CopyVertexAttributes = [&gltf, &buffers, &s, &i](s64 accIndex, s64 offset)
				{
					auto acc = &gltf.accessors[accIndex];
					auto bv = &gltf.bufferViews[acc->bufferView];
					auto b = (V3 *)(buffers[bv->buffer].elements + bv->byteOffset + acc->byteOffset);
					auto dst = (V3 *)(((u8 *)s.Map()) + offset);
					auto ofs = V3{};
					if (offset == 0)
					{
						//auto r = (f32)MeshCount / 10;
						auto r = (f32)1;
						ofs.x = i * 10.0f;
						ofs.y = i * 0.0f;
						ofs.z = i * 0.0f;
						//ofs.x = ((float)rand() * 2/(float)((f32)RAND_MAX/r)) - ((float)r / 2);
						//ofs.y = ((float)rand() * 2/(float)((f32)RAND_MAX/r)) - ((float)r / 2);
						//ofs.z = ((float)rand() * 2/(float)((f32)RAND_MAX/r)) - ((float)r / 2);
					}
					PrintV3(ofs);
					for (auto i = 0; i < acc->count; i += 1)
					{
						auto v = *b;
						//v.x += ofs.x;
						//v.y += ofs.y;
						//v.z += ofs.z;
						*dst = v;
						dst += 2;
						b += 1;
					}
				};
				CopyVertexAttributes(posAccessIndex, 0);
				CopyVertexAttributes(normAccessIndex, sizeof(V3));
				s.FlushIn(cb);
				//m->FlushVertexBuffer();
			}
		}
		}
	}
	cb.Queue();
	return {};
}

ModelAsset LoadModelAsset(String name)
{
	if (DevelopmentBuild)
	{
		LoadModelAssetFromFile(name);
	}
	else
	{
		Abort("Model", "@TODO: Load model assets in release mode.");
	}
	return {};
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
