#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// @TODO
#define ASSET_COUNT 100

struct AssetsContext
{
	void *lookup[ASSET_COUNT]; // @TODO: Use a hash table.
	AssetLoadStatus loadStatuses[ASSET_COUNT];
	//AssetUploadDoubleBuffer uploadCommandBuffers;
	//s32 uploadFenceCount;
	//GfxFence uploadFences[MAX_UPLOAD_COMMAND_BUFFERS];
	//s32 pendingLoadStatusCounts[MAX_UPLOAD_FENCES];
	//AssetLoadStatus *pendingLoadStatuses[MAX_UPLOAD_FENCES][MAX_UPLOAD_COMMAND_BUFFERS];
} assetsContext;

// @TODO: Trying to load the same asset multiple times simultaneously?

GfxCommandBuffer theBuffer;
bool ready = false;

GPUIndexedGeometry QueueIndexedGeometryUploadToGPU(u32 verticesByteSize, u32 indicesByteSize, GfxBuffer vertexStagingBuffer, AssetLoadStatus *loadStatus)
{
	auto vertexBuffer = CreateGPUBuffer(verticesByteSize, GFX_VERTEX_BUFFER | GFX_TRANSFER_DESTINATION_BUFFER, GFX_DEVICE_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT);
	auto indexBuffer = CreateGPUBuffer(indicesByteSize, GFX_INDEX_BUFFER | GFX_TRANSFER_DESTINATION_BUFFER, GFX_DEVICE_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT);
	//auto commandBuffer = CreateUploadCommandBuffer();
	theBuffer = CreateGPUCommandBuffer(GFX_TRANSFER_COMMAND_QUEUE, GPU_RESOURCE_LIFETIME_PERSISTENT);
	GfxRecordCopyBufferCommand(theBuffer, verticesByteSize, vertexStagingBuffer, vertexBuffer, 0, 0);
	GfxRecordCopyBufferCommand(theBuffer, indicesByteSize, vertexStagingBuffer, indexBuffer, verticesByteSize, 0);
	GfxEndCommandBuffer(theBuffer);
	QueueGPUTransferCommandBuffer(theBuffer);
	ready = true;
	//auto fence = GfxCreateFence(false);
	//GfxSubmitCommandBuffers(1, &theBuffer, GFX_GRAPHICS_COMMAND_QUEUE, fence);
	return
	{
		.vertexBuffer = vertexBuffer,
		.indexBuffer = indexBuffer,
	};
}

u32 QueueTextureUploadToGPU(u8 *pixels, s32 texturePixelWidth, s32 texturePixelHeight, AssetLoadStatus *loadStatus)
{
	// @TODO: Load texture directly into staging memory.
	void *stagingMemory;
	u32 textureByteSize = sizeof(u32) * texturePixelWidth * texturePixelHeight;
	auto stagingBuffer = CreateGPUBuffer(textureByteSize, GFX_TRANSFER_SOURCE_BUFFER, GFX_HOST_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT, &stagingMemory);
	CopyMemory(pixels, stagingMemory, textureByteSize);
	auto image = CreateGPUImage(texturePixelWidth, texturePixelHeight, GFX_FORMAT_R8G8B8A8_UNORM, GFX_IMAGE_LAYOUT_UNDEFINED, GFX_IMAGE_USAGE_TRANSFER_DST | GFX_IMAGE_USAGE_SAMPLED, GFX_SAMPLE_COUNT_1);
	auto commandBuffer = CreateGPUCommandBuffer(GFX_TRANSFER_COMMAND_QUEUE, GPU_RESOURCE_LIFETIME_PERSISTENT);
	GfxTransitionImageLayout(commandBuffer, image, GFX_FORMAT_R8G8B8A8_UNORM, GFX_IMAGE_LAYOUT_UNDEFINED, GFX_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	GfxRecordCopyBufferToImageCommand(commandBuffer, stagingBuffer, image, texturePixelWidth, texturePixelHeight);
	GfxTransitionImageLayout(commandBuffer, image, GFX_FORMAT_R8G8B8A8_UNORM, GFX_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, GFX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	GfxEndCommandBuffer(commandBuffer);
	QueueGPUTransferCommandBuffer(commandBuffer);
	return 0;
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
		LogPrint(LogType::ERROR, "failed to load texture %s\n", &jobParameter->path[0]);
		return;
	}
	Defer(free(pixels));
	*jobParameter->outputTextureID = QueueTextureUploadToGPU(pixels, texturePixelWidth, texturePixelHeight, jobParameter->loadStatus);
}

// @TODO @PREPROCESSOR: Generate these.
struct {
	AssetID asset_id;
	const char *filepath;
} asset_id_to_filepath_map[] = {
	{ANVIL_ASSET, "Data/Model/Anvil"},
	{GUY_ASSET, "Data/Model/Guy"},
	{SPONZA_ASSET, "Data/Model/Sponza"},
};

struct LoadModelJobParameter {
	AssetID assetID;
	void **outputMeshAssetAddress;
};

void LoadModel(void *jobParameterPointer) {
	LoadModelJobParameter *jobParameter = (LoadModelJobParameter *)jobParameterPointer;

	MeshAsset *mesh = (MeshAsset *)malloc(sizeof(MeshAsset));

	String modelDirectory;
	bool foundModel = false;
	for (auto i = 0; i < ArrayCount(asset_id_to_filepath_map); i++)
	{
		if (asset_id_to_filepath_map[i].asset_id == jobParameter->assetID)
		{
			modelDirectory = asset_id_to_filepath_map[i].filepath;
			foundModel = true;
			break;
		}
	}
	Assert(foundModel);

	String modelName = GetFilepathFilename(modelDirectory);
	String fbxFilename = "test2.fbx";//Concatenate(modelName, ".fbx");
	String fbxFilepath = JoinFilepaths(modelDirectory, fbxFilename);
	auto assimpScene = aiImportFile(&fbxFilepath[0], aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_RemoveRedundantMaterials);
	if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
	{
		Abort("assimp error: %s", aiGetErrorString());
	}

	//u32 submeshCount = assimpScene->mNumMeshes;
	auto submeshCount = 1;

	Resize(&mesh->submeshes, submeshCount);
	Resize(&mesh->materials, submeshCount);
	for (auto i = 0; i < submeshCount; i++)
	{
		LoadTextureJobParameter *loadTextureJobParameters = (LoadTextureJobParameter *)malloc(5 * sizeof(LoadTextureJobParameter)); // @TODO
		loadTextureJobParameters[0] =
		LoadTextureJobParameter{
			.path = JoinFilepaths(modelDirectory, "albedo.png"),
			.outputTextureID = &mesh->materials[i].albedo_map,
		};
		loadTextureJobParameters[1] =
		LoadTextureJobParameter{
			.path = JoinFilepaths(modelDirectory, "normal.png"),
			.outputTextureID = &mesh->materials[i].normal_map,
		};
		loadTextureJobParameters[2] =
		LoadTextureJobParameter{
			.path = JoinFilepaths(modelDirectory, "roughness.png"),
			.outputTextureID = &mesh->materials[i].roughness_map,
		};
		loadTextureJobParameters[3] =
		LoadTextureJobParameter{
			.path = JoinFilepaths(modelDirectory, "metallic.png"),
			.outputTextureID = &mesh->materials[i].metallic_map,
		};
		loadTextureJobParameters[4] =
		LoadTextureJobParameter{
			.path = JoinFilepaths(modelDirectory, "ambient_occlusion.png"),
			.outputTextureID = &mesh->materials[i].ambient_occlusion_map,
		};
		JobDeclaration loadTextureJobDeclarations[5]; // @TODO
		for (auto j = 0; j < ArrayCount(loadTextureJobDeclarations); j++)
		{
			loadTextureJobDeclarations[j] = CreateJob(LoadTexture, &loadTextureJobParameters[j]);
		}
		RunJobs(ArrayCount(loadTextureJobDeclarations), loadTextureJobDeclarations, NORMAL_PRIORITY_JOB, NULL);
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
	auto stagingBuffer = CreateGPUBuffer(verticesSize + indicesSize, GFX_TRANSFER_SOURCE_BUFFER, GFX_HOST_MEMORY, GPU_RESOURCE_LIFETIME_PERSISTENT, &stagingMemory);

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
#if 0
		if (assimp_mesh->mTextureCoords[0]) {
			aiString diffuse_path, specular_path; // Relative to the fbx file's directory.
			aiReturn has_diffuse = mat->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_path);
			if (has_diffuse == aiReturn_SUCCESS) {
				auto texture_path = join_paths(model_directory, diffuse_path.C_Str());
				mesh->texture_id = load_texture(texture_path);
			} else {
				asset->material = UNTEXTURED_MATERIAL;
			}
		}
#endif

#if 0

	//glUseProgram(animated_mesh_shader);

	//model->meshes.resize(assimp_scene->mNumMeshes);

	//models_.lookup[id] = models_.instances.count;
	//Model_ *model_ = array_add(&models_.instances, 1);
	//allocate_array(&model_->meshes, assimp_scene->mNumMeshes);
	//model->num_meshes = assimp_scene->mNumMeshes;
	//model->meshes.resize(model->mesh_count);

	//if (num_joints == 0) {
		for (s32 i = 0; i < assimp_scene->mNumMeshes; i++) {
			assert(i == 0);

			aiMesh* assimp_mesh = assimp_scene->mMeshes[i];
			assert(assimp_mesh->HasPositions() && assimp_mesh->HasNormals() && assimp_mesh->HasFaces());

			//Mesh *mesh = &model->meshes[i];
			vulkan_context.vertices.resize(assimp_mesh->mNumVertices);
			vulkan_context.indices.resize(assimp_mesh->mNumFaces * 3);
			//glGenVertexArrays(1, &mesh->vao);
			//glBindVertexArray(mesh->vao);

			//Static_Array<Vertex> vertex_buffer;
			//allocate_array(&vertex_buffer, assimp_mesh->mNumVertices);
			//Vertex *vertex_buffer = (Vertex *)malloc(sizeof(Vertex) * assimp_mesh->mNumVertices);

			for (s32 j = 0; j < assimp_mesh->mNumVertices; j++) {
				Vertex *v = &vulkan_context.vertices[j];

				v->position[0] = assimp_mesh->mVertices[j].x;
				v->position[1] = assimp_mesh->mVertices[j].y;
				v->position[2] = assimp_mesh->mVertices[j].z;

				if (assimp_mesh->mNormals) {
					v->normal[0] = assimp_mesh->mNormals[j].x;
					v->normal[1] = assimp_mesh->mNormals[j].y;
					v->normal[2] = assimp_mesh->mNormals[j].z;
				}
				if (assimp_mesh->mTextureCoords[0]) {
					v->uv[0] = assimp_mesh->mTextureCoords[0][j].x;
					v->uv[1] = assimp_mesh->mTextureCoords[0][j].y;
				}
			}

			//glGenBuffers(1, &mesh->vbo);
			//glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
			//glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * assimp_mesh->mNumVertices, vertex_buffer.data, GL_DYNAMIC_DRAW);
			//free_array(&vertex_buffer);

			//mesh->index_count = assimp_mesh->mNumFaces * 3;
			//Static_Array<u32> index_buffer;
			//allocate_array(&index_buffer, mesh->index_count);

			for (s32 j = 0; j < assimp_mesh->mNumFaces; j++) {
				aiFace assimp_face = assimp_mesh->mFaces[j];

				assert(assimp_face.mNumIndices == 3);

				for (s32 k = 0; k < assimp_face.mNumIndices; k++) {
					vulkan_context.indices[(3 * j) + k] = assimp_face.mIndices[k];
				}
			}
#endif
			//glGenBuffers(1, &mesh->ebo);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
			//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * mesh->index_count, index_buffer.data, GL_DYNAMIC_DRAW);
			//free_array(&index_buffer);

			// Doesn't handle multiple textures per mesh.
			/*
			mesh->texture_id = 0;
			aiMaterial* mat = assimp_scene->mMaterials[assimp_mesh->mMaterialIndex];
			aiString diffuse_path, specular_path; // Relative to the fbx file's directory.
			aiReturn has_diffuse = mat->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_path);
			if (has_diffuse == aiReturn_SUCCESS) {
				auto texture_path = join_paths(model_directory, diffuse_path.C_Str());
				mesh->texture_id = load_texture(texture_path);
			}
			*/

			/*
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, position));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, normal));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, uv));

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			*/
		/*
		}
	} else {
		skeletons_.lookup[id] = skeletons_.instances.count;

		Skeleton_Instance *skeleton_instance = array_add(&skeletons_.instances, 1);
		allocate_array(&skeleton_instance->local_joint_poses, num_joints);
		allocate_array(&skeleton_instance->global_joint_poses, num_joints);

		Skeleton_Asset *skeleton_asset = array_add(&skeletons_.assets, 1);
		skeleton_asset->num_joints = num_joints;
		allocate_array(&skeleton_asset->names, num_joints);
		allocate_array(&skeleton_asset->skinning_info , num_joints);
		allocate_array(&skeleton_asset->parent_indices, num_joints);
		allocate_array(&skeleton_asset->inverse_rest_pose, num_joints);

		s32 num_leaf_nodes = 0;
		for (s32 i = 0; i < assimp_scene->mNumMeshes; i++) {
			aiMesh* assimp_mesh = assimp_scene->mMeshes[i];

			for (s32 j = 0; j < assimp_mesh->mNumBones; j++) {
				aiNode *n = assimp_scene->mRootNode->FindNode(assimp_mesh->mBones[j]->mName);

				for (s32  k = 0; k < n->mNumChildren; k++) {
					if (n->mChildren[k]->mName == aiString(std::string(n->mName.C_Str()) + "_end")) {
						num_leaf_nodes += 1;
					}
				}
			}
		}

		skeleton_asset->num_leaf_nodes = num_leaf_nodes;
		allocate_array(&skeleton_asset->leaf_node_parent_indices, num_leaf_nodes);
		allocate_array(&skeleton_asset->leaf_node_translations, num_leaf_nodes);
		skeleton_asset->parent_indices[0] = UINT8_MAX;

		skeleton_instance->asset = skeleton_asset;

		s32 joint_base_index = 0;

		for (s32 i = 0; i < assimp_scene->mNumMeshes; i++) {
			aiMesh* assimp_mesh = assimp_scene->mMeshes[i];
			assert(assimp_mesh->HasPositions() && assimp_mesh->HasNormals() && assimp_mesh->HasFaces());

			Mesh_ *mesh = &model_->meshes[i];

			glGenVertexArrays(1, &mesh->vao);
			glBindVertexArray(mesh->vao);

			Static_Array<Skinned_Vertex> vertex_buffer;
			allocate_array(&vertex_buffer, assimp_mesh->mNumVertices);

			for (s32 j = 0; j < assimp_mesh->mNumVertices; j++) {
				Skinned_Vertex *v = &vertex_buffer[j];

				v->position[0] = assimp_mesh->mVertices[j].x;
				v->position[1] = assimp_mesh->mVertices[j].y;
				v->position[2] = assimp_mesh->mVertices[j].z;

				v->joint_indices[0] = 0;
				v->joint_indices[1] = 0;
				v->joint_indices[2] = 0;
				v->joint_indices[3] = 0;

				v->weights[0] = 0;
				v->weights[1] = 0;
				v->weights[2] = 0;
				v->weights[3] = 0;

				if (assimp_mesh->mNormals) {
					v->normal[0] = assimp_mesh->mNormals[j].x;
					v->normal[1] = assimp_mesh->mNormals[j].y;
					v->normal[2] = assimp_mesh->mNormals[j].z;
				}
				if (assimp_mesh->mTextureCoords[0]) {
					v->uv[0] = assimp_mesh->mTextureCoords[0][j].x;
					v->uv[1] = assimp_mesh->mTextureCoords[0][j].y;
				}
			}

			for (s32 j = 0; j < assimp_mesh->mNumBones; j++) {
				s32 joint_index = joint_base_index + j;

				skeleton_asset->names[joint_index] = assimp_mesh->mBones[j]->mName;
				allocate_array(&skeleton_asset->skinning_info[joint_index].vertices, assimp_mesh->mBones[j]->mNumWeights);
				allocate_array(&skeleton_asset->skinning_info[joint_index].weights , assimp_mesh->mBones[j]->mNumWeights);

				for (s32 k = 0; k < assimp_mesh->mBones[j]->mNumWeights; k++) {
					skeleton_asset->skinning_info[joint_index].vertices[k] = assimp_mesh->mBones[j]->mWeights[k].mVertexId;
					skeleton_asset->skinning_info[joint_index].weights[k] = assimp_mesh->mBones[j]->mWeights[k].mWeight;

					u32 id = assimp_mesh->mBones[j]->mWeights[k].mVertexId;
					for (s32 l = 0; l < 4; l++) {
						if (vertex_buffer[id].weights[l] == 0) {
							vertex_buffer[id].joint_indices[l] = j;
							vertex_buffer[id].weights[l] = assimp_mesh->mBones[j]->mWeights[k].mWeight;
							break;
						}
					}

					skeleton_asset->inverse_rest_pose[joint_index] = assimp_matrix_to_m4(&assimp_mesh->mBones[j]->mOffsetMatrix);
				}

				aiNode *bone_node = assimp_scene->mRootNode->FindNode(skeleton_asset->names[joint_index]);
				assert(bone_node != NULL);

				if (joint_index == 0) {
					skeleton_instance->global_joint_poses[joint_index] = assimp_matrix_to_m4(&bone_node->mTransformation);
					continue;
				}

				if (bone_node->mParent == NULL) {
					continue;
				}

				for (s32 k = 0; k < joint_index; k++) {
					if (skeleton_asset->names[k] == bone_node->mParent->mName) {
						skeleton_asset->parent_indices[joint_index] = k;
						skeleton_instance->global_joint_poses[joint_index] = skeleton_instance->global_joint_poses[k] * assimp_matrix_to_m4(&bone_node->mTransformation);
						break;
					}
				}
			}

			joint_base_index += assimp_mesh->mNumBones;

			glGenBuffers(1, &mesh->vbo);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Skinned_Vertex) * assimp_mesh->mNumVertices, vertex_buffer.data, GL_DYNAMIC_DRAW);
			free_array(&vertex_buffer);

			mesh->index_count = assimp_mesh->mNumFaces * 3;
			Static_Array<u32> index_buffer;
			allocate_array(&index_buffer, mesh->index_count);

			for (s32 j = 0; j < assimp_mesh->mNumFaces; j++) {
				aiFace assimp_face = assimp_mesh->mFaces[j];

				assert(assimp_face.mNumIndices == 3);

				for (s32 k = 0; k < assimp_face.mNumIndices; k++) {
					index_buffer[(3 * j) + k] = assimp_face.mIndices[k];
				}
			}

			glGenBuffers(1, &mesh->ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * mesh->index_count, index_buffer.data, GL_DYNAMIC_DRAW);
			free_array(&index_buffer);

			// Doesn't handle multiple textures per mesh.
			mesh->texture_id = 0;
			aiMaterial* mat = assimp_scene->mMaterials[assimp_mesh->mMaterialIndex];
			aiString diffuse_path, specular_path; // Relative to the fbx file's directory.
			aiReturn has_diffuse = mat->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_path);
			if (has_diffuse == aiReturn_SUCCESS) {
				auto texture_path = join_paths(model_directory, diffuse_path.C_Str());
				mesh->texture_id = load_texture(texture_path);
			}

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Skinned_Vertex), (GLvoid *)offsetof(Skinned_Vertex, position));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Skinned_Vertex), (GLvoid *)offsetof(Skinned_Vertex, normal));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Skinned_Vertex), (GLvoid *)offsetof(Skinned_Vertex, uv));
			glVertexAttribIPointer(3, 4, GL_INT, sizeof(Skinned_Vertex), (GLvoid *)offsetof(Skinned_Vertex, joint_indices));
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Skinned_Vertex), (GLvoid *)offsetof(Skinned_Vertex, weights));

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			glEnableVertexAttribArray(4);
		}
	}
	*/
}

// @TODO: Shouldn't we call Get_Model_Asset?
MeshAsset *GetMeshAsset(AssetID assetID)
{
	if (assetsContext.lookup[assetID])
	{
		return (MeshAsset *)assetsContext.lookup[assetID];
	}

	// @TODO: Call load model asset. No outputMeshAssetAddress.
	LoadModelJobParameter *jobParameter = (LoadModelJobParameter *)malloc(sizeof(LoadModelJobParameter)); // @TODO
	jobParameter->assetID = assetID;
	jobParameter->outputMeshAssetAddress = &assetsContext.lookup[assetID];
	JobDeclaration jobDeclaration = CreateJob(LoadModel, jobParameter);
	JobCounter jobCounter;
	RunJobs(1, &jobDeclaration, NORMAL_PRIORITY_JOB, &jobCounter);
	WaitForJobCounter(&jobCounter); // @TODO: Get rid of this wait!

	return (MeshAsset *)assetsContext.lookup[assetID];
}

bool IsAssetLoaded(AssetID assetID)
{
	return assetsContext.loadStatuses[assetID] == ASSET_LOADED;
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
			*assetsContext.uploadCommandBuffers.writeBuffer->loadStatuses[0] = ASSET_LOADED;
		}
	}
	return;

	// Submit queued asset upload commands.
	SwitchAssetUploadDoubleBuffer(&assetsContext.uploadCommandBuffers);
	if (assetsContext.uploadCommandBuffers.readBufferElementCount > 0)
	{
		ConsolePrint("READING FROM: %x\n", assetsContext.uploadCommandBuffers.readBuffer->commandBuffers);
		ConsolePrint("c: %u\n", assetsContext.uploadCommandBuffers.readBufferElementCount);
		GfxSubmitCommandBuffers(assetsContext.uploadCommandBuffers.readBufferElementCount, assetsContext.uploadCommandBuffers.readBuffer->commandBuffers, GFX_GRAPHICS_COMMAND_QUEUE, assetsContext.uploadFences[assetsContext.uploadFenceCount]);
		assetsContext.pendingLoadStatusCounts[assetsContext.uploadFenceCount] = assetsContext.uploadCommandBuffers.readBufferElementCount;
		CopyMemory(assetsContext.uploadCommandBuffers.readBuffer->loadStatuses, &assetsContext.pendingLoadStatuses[assetsContext.uploadFenceCount], assetsContext.uploadCommandBuffers.readBufferElementCount * sizeof(AssetLoadStatus *));
		assetsContext.uploadFenceCount += 1;
		// @TODO: Render_API_Free_Command_Buffers(assetsContext.gpu_upload_command_pool, assetsContext.uploadCommandBuffers.readBufferElementCount, assetsContext.uploadCommandBuffers.read_buffer->command_buffers);
	}

	// Check whether submitted asset upload commands have finished.
	for (auto i = 0; i < assetsContext.uploadFenceCount; i++)
	{
		if (!GfxWasFenceSignalled(assetsContext.uploadFences[i]))
		{
			continue;
		}
		GfxResetFences(1, &assetsContext.uploadFences[i]);
		for (auto j = 0; j < assetsContext.pendingLoadStatusCounts[i]; j++)
		{
			*assetsContext.pendingLoadStatuses[i][j] = ASSET_LOADED;
		}
		// Remove the fence.
		assetsContext.uploadFenceCount -= 1;
		if (i != assetsContext.uploadFenceCount)
		{
			GfxFence temporary = assetsContext.uploadFences[i];
			assetsContext.uploadFences[i] = assetsContext.uploadFences[assetsContext.uploadFenceCount];
			assetsContext.uploadFences[assetsContext.uploadFenceCount] = temporary;
			i -= 1;
		}
	}
}
#endif

void InitializeAssets(void *job_parameter)
{
	for (s32 i = 0; i < MAX_UPLOAD_FENCES; i++)
	{
		assetsContext.uploadFences[i] = GfxCreateFence(false);
	}
}
