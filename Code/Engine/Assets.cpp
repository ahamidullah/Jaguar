#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Assets.h"

constexpr s32 MAX_UPLOAD_COMMAND_BUFFERS = 128;
constexpr s32 MAX_UPLOAD_FENCES = 32;

struct AssetUploadCounter
{
	s32 commandBufferCount;
	AssetLoadStatus *loadStatus;
};

struct AssetUploadDoubleBuffer
{
	struct UploadBuffer
	{
		s32 readyCount = 0;
		GPU_Command_Buffer commandBuffers[MAX_UPLOAD_COMMAND_BUFFERS];
		AssetUploadCounter *uploadCounters[MAX_UPLOAD_COMMAND_BUFFERS];
	};
	UploadBuffer buffers[2];
	GPU_Command_Buffer *writeHead = buffers[0].commandBuffers;
	UploadBuffer *writeBuffer = &buffers[0];
	UploadBuffer *readBuffer = &buffers[1];
	s32 readBufferElementCount = 0;
};

void WriteToAssetUploadDoubleBuffer(AssetUploadDoubleBuffer *b, GPU_Command_Buffer commandBuffer, AssetUploadCounter *counter)
{
	GPU_Command_Buffer *writePointer;
	do
	{
		writePointer = b->writeHead;
		AtomicCompareAndSwap((void *volatile *)&b->writeHead, writePointer, writePointer + 1);
	} while (b->writeHead != writePointer + 1);
	*writePointer = commandBuffer;
	s32 bufferIndex = (writePointer < b->buffers[1].commandBuffers) ? 0 : 1;
	s32 elementIndex = writePointer - b->buffers[bufferIndex].commandBuffers;
	Assert(elementIndex < MAX_UPLOAD_COMMAND_BUFFERS);
	b->buffers[bufferIndex].uploadCounters[elementIndex] = counter;
	b->buffers[bufferIndex].readyCount += 1;
}

void SwitchAssetUploadDoubleBuffer(AssetUploadDoubleBuffer *b)
{
	if (b->writeBuffer == &b->buffers[0])
	{
		b->writeBuffer = &b->buffers[1];
		b->readBuffer = &b->buffers[0];
	}
	else
	{
		b->writeBuffer = &b->buffers[0];
		b->readBuffer = &b->buffers[1];
	}
	GPU_Command_Buffer *oldWriteHead = b->writeHead;
	b->writeBuffer->readyCount = 0;
	b->writeHead = b->writeBuffer->commandBuffers;
	b->readBufferElementCount = oldWriteHead - b->readBuffer->commandBuffers;
	while (b->readBufferElementCount != b->readBuffer->readyCount)
	{
		//
	}
}

// @TODO
#define ASSET_COUNT 100

struct
{
	void *lookup[ASSET_COUNT]; // @TODO: Use a hash table.
	AssetLoadStatus loadStatuses[ASSET_COUNT];
	AssetUploadDoubleBuffer uploadCommandBuffers;
	s32 uploadFenceCount;
	GPU_Fence uploadFences[MAX_UPLOAD_COMMAND_BUFFERS];
	s32 pendingUploadCounterCounts[MAX_UPLOAD_FENCES];
	AssetUploadCounter *pendingUploadCountersPerFence[MAX_UPLOAD_FENCES][MAX_UPLOAD_COMMAND_BUFFERS];
	GPU_Command_Pool gpu_upload_command_pool;
} assetsContext;

// @TODO: Trying to load the same asset multiple times simultaneously?

//#define DEFAULT_DIFFUSE_COLOR (V3){1.0f, 0.08f, 0.58f}
//#define DEFAULT_DIFFUSE_COLOR (V3){0.0f, 1.00f, 0.00f}
//#define DEFAULT_SPECULAR_COLOR (V3){1.0f, 1.0f, 1.0f}

Renderer::GPUIndexedGeometry QueueIndexedGeometryUploadToGPU(u32 verticesByteSize, u32 indicesByteSize, GPU_Buffer sourceBuffer, AssetUploadCounter *counter) {
	GPU_Buffer vertexBuffer = Renderer::CreateGPUBuffer(verticesByteSize, (GPU_Buffer_Usage_Flags)(GPU_VERTEX_BUFFER | GPU_TRANSFER_DESTINATION_BUFFER));
	GPU_Buffer indexBuffer = Renderer::CreateGPUBuffer(indicesByteSize, (GPU_Buffer_Usage_Flags)(GPU_INDEX_BUFFER | GPU_TRANSFER_DESTINATION_BUFFER));
	GPU_Command_Buffer commandBuffer = Render_API_Create_Command_Buffer(renderContext.thread_local_contexts[threadIndex].upload_command_pool);
	Render_API_Record_Copy_Buffer_Command(commandBuffer, verticesByteSize, sourceBuffer, vertexBuffer, 0, 0);
	Render_API_Record_Copy_Buffer_Command(commandBuffer, indicesByteSize, sourceBuffer, indexBuffer, verticesByteSize, 0);
	Render_API_End_Command_Buffer(commandBuffer);
	WriteToAssetUploadDoubleBuffer(&assetsContext.uploadCommandBuffers, commandBuffer, counter);
	return {
		.vertex_buffer = vertexBuffer,
		.index_buffer = indexBuffer,
	};
}

u32 QueueTextureUploadToGPU(u8 *pixels, s32 texturePixelWidth, s32 texturePixelHeight, AssetUploadCounter *counter) {
	// @TODO: Load texture directly into staging memory.
	void *stagingMemory;
	u32 textureByteSize = sizeof(u32) * texturePixelWidth * texturePixelHeight;
	GPU_Buffer stagingBuffer = Renderer::CreateGPUStagingBuffer(textureByteSize, &stagingMemory);
	CopyMemory(pixels, stagingMemory, textureByteSize);
	GPU_Image image = Renderer::CreateGPUImage(texturePixelWidth, texturePixelHeight, GPU_FORMAT_R8G8B8A8_UNORM, GPU_IMAGE_LAYOUT_UNDEFINED, (GPU_Image_Usage_Flags)(GPU_IMAGE_USAGE_TRANSFER_DST | GPU_IMAGE_USAGE_SAMPLED), GPU_SAMPLE_COUNT_1);
	GPU_Command_Buffer commandBuffer = Render_API_Create_Command_Buffer(renderContext.thread_local_contexts[threadIndex].upload_command_pool);
	Render_API_Transition_Image_Layout(commandBuffer, image, (VkFormat)GPU_FORMAT_R8G8B8A8_UNORM, (VkImageLayout)GPU_IMAGE_LAYOUT_UNDEFINED, (VkImageLayout)GPU_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Render_API_Record_Copy_Buffer_To_Image_Command(commandBuffer, stagingBuffer, image, texturePixelWidth, texturePixelHeight);
	Render_API_Transition_Image_Layout(commandBuffer, image, (VkFormat)GPU_FORMAT_R8G8B8A8_UNORM, (VkImageLayout)GPU_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (VkImageLayout)GPU_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	Render_API_End_Command_Buffer(commandBuffer);
	WriteToAssetUploadDoubleBuffer(&assetsContext.uploadCommandBuffers, commandBuffer, counter);
	return 0;
}

struct LoadTextureJobParameter {
	String path;
	AssetUploadCounter *uploadCounter;
	Texture_ID *outputTextureID;
};

void LoadTexture(void *jobParameterPointer) {
	LoadTextureJobParameter *jobParameter = (LoadTextureJobParameter *)jobParameterPointer;
	s32 texturePixelWidth, texturePixelHeight, textureChannels;
	u8 *pixels = stbi_load(&jobParameter->path[0], &texturePixelWidth, &texturePixelHeight, &textureChannels, STBI_rgb_alpha);
	Assert(pixels);
	Defer(free(pixels));
	*jobParameter->outputTextureID = QueueTextureUploadToGPU(pixels, texturePixelWidth, texturePixelHeight, jobParameter->uploadCounter);
}

// @TODO @PREPROCESSOR: Generate these.
struct {
	AssetID asset_id;
	const char *filepath;
} asset_id_to_filepath_map[] = {
	{ANVIL_ASSET, "data/models/anvil"},
	{GUY_ASSET, "data/models/guy"},
};

struct LoadModelJobParameter {
	AssetID assetID;
	void **outputMeshAssetAddress;
};

void LoadModel(void *jobParameterPointer) {
	LoadModelJobParameter *jobParameter = (LoadModelJobParameter *)jobParameterPointer;

	MeshAsset *mesh = (MeshAsset *)malloc(sizeof(MeshAsset));

	AssetUploadCounter *uploadCounter = (AssetUploadCounter *)malloc(sizeof(AssetUploadCounter));
	uploadCounter->loadStatus = &mesh->loadStatus;
	uploadCounter->commandBufferCount = 6; // @TODO

	String modelDirectory = {};
	bool foundModel = false;
	for (u32 i = 0; i < ArrayCount(asset_id_to_filepath_map); i++) {
		if (asset_id_to_filepath_map[i].asset_id == jobParameter->assetID) {
			modelDirectory = asset_id_to_filepath_map[i].filepath;
			foundModel = true;
			break;
		}
	}
	Assert(foundModel);

	String modelName = GetFilepathFilename(modelDirectory);
	String fbxFilename = Concatenate(modelName, ".fbx");
	String fbxFilepath = JoinFilepaths(modelDirectory, fbxFilename);
	const struct aiScene* assimp_scene = aiImportFile(&fbxFilepath[0], aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_RemoveRedundantMaterials);
	if (!assimp_scene || assimp_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimp_scene->mRootNode) {
		Abort("assimp error: %s", aiGetErrorString());
	}

	s32 num_joints = 0;
	for (s32 i = 0; i < assimp_scene->mNumMeshes; i++) {
		num_joints += assimp_scene->mMeshes[i]->mNumBones;
	}

	u32 submesh_count = assimp_scene->mNumMeshes;

	//mesh->materials = (Material *)malloc(sizeof(Material) * submesh_count); // @TODO
	Resize(&mesh->materials, submesh_count);
	for (s32 i = 0; i < submesh_count; i++) {
		LoadTextureJobParameter *load_texture_jobParameters = (LoadTextureJobParameter *)malloc(5 * sizeof(LoadTextureJobParameter)); // @TODO
		load_texture_jobParameters[0] = (LoadTextureJobParameter){
			.path = JoinFilepaths(modelDirectory, "albedo.png"),
			.uploadCounter = uploadCounter,
			.outputTextureID = &mesh->materials[i].albedo_map,
		};
		load_texture_jobParameters[1] = (LoadTextureJobParameter){
			.path = JoinFilepaths(modelDirectory, "normal.png"),
			.uploadCounter = uploadCounter,
			.outputTextureID = &mesh->materials[i].normal_map,
		};
		load_texture_jobParameters[2] = (LoadTextureJobParameter){
			.path = JoinFilepaths(modelDirectory, "roughness.png"),
			.uploadCounter = uploadCounter,
			.outputTextureID = &mesh->materials[i].roughness_map,
		};
		load_texture_jobParameters[3] = (LoadTextureJobParameter){
			.path = JoinFilepaths(modelDirectory, "metallic.png"),
			.uploadCounter = uploadCounter,
			.outputTextureID = &mesh->materials[i].metallic_map,
		};
		load_texture_jobParameters[4] = (LoadTextureJobParameter){
			.path = JoinFilepaths(modelDirectory, "ambient_occlusion.png"),
			.uploadCounter = uploadCounter,
			.outputTextureID = &mesh->materials[i].ambient_occlusion_map,
		};
		JobDeclaration load_texture_job_declarations[5]; // @TODO
		for (s32 j = 0; j < ArrayCount(load_texture_job_declarations); j++) {
			load_texture_job_declarations[j] = CreateJob(LoadTexture, &load_texture_jobParameters[j]);
		}
		RunJobs(ArrayCount(load_texture_job_declarations), load_texture_job_declarations, NORMAL_PRIORITY_JOB, NULL);
	}

	u32 vertex_count = 0;
	u32 index_count = 0;
	for (s32 i = 0; i < submesh_count; i++) {
		vertex_count += assimp_scene->mMeshes[i]->mNumVertices;
		index_count += assimp_scene->mMeshes[i]->mNumFaces * 3;
	}

	mesh->vertex_count = vertex_count;
	//mesh->vertices = malloc(sizeof(Vertex) * mesh->vertex_count);
	mesh->index_count = index_count;
	//mesh->indices = malloc(sizeof(u32) * mesh->index_count);
	//mesh->submesh_count = submesh_count;
	//mesh->submesh_index_counts = (u32 *)malloc(sizeof(u32) * mesh->submesh_count);; // @TODO
	Resize(&mesh->submeshIndexCounts, submesh_count);
	//jobParameter->assetsContext.lookup[asset_id] = mesh;
	*(jobParameter->outputMeshAssetAddress) = mesh;

	u32 mesh_vertex_offset = 0;
	u32 mesh_index_offset = 0;
	u32 max_x_vertex = 0, max_y_vertex = 0, max_z_vertex = 0;
	u32 min_x_vertex = 0, min_y_vertex = 0, min_z_vertex = 0;
	for (u32 i = 0; i < submesh_count; i++) {
		struct aiMesh *assimp_mesh = assimp_scene->mMeshes[i];
		assert(assimp_mesh->mVertices && assimp_mesh->mNormals && (assimp_mesh->mFaces && assimp_mesh->mNumFaces > 0));

		//u32 vertices_size = sizeof(Vertex) * assimp_mesh->mNumVertices;
		u32 vertices_size = sizeof(V3) * assimp_mesh->mNumVertices;
		u32 indices_size = sizeof(u32) * 3 * assimp_mesh->mNumFaces;
		//Vertex *vertex_buffer = malloc(vertices_size); // @TODO
		V3 *vertex_buffer = (V3 *)malloc(vertices_size); // @TODO
		u32 *index_buffer = (u32 *)malloc(indices_size); // @TODO

		for (s32 j = 0; j < assimp_mesh->mNumVertices; j++) {
			//Vertex *v = &vertex_buffer[mesh_vertex_offset + j];
			V3 *v = &vertex_buffer[mesh_vertex_offset + j];
			//v->position = (V3){
			*v = {
				.X = assimp_mesh->mVertices[j].x,
				.Y = assimp_mesh->mVertices[j].y,
				.Z = assimp_mesh->mVertices[j].z,
			};
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

		for (s32 j = 0; j < assimp_mesh->mNumFaces; j++) {
			assert(assimp_mesh->mFaces[j].mNumIndices == 3);
			index_buffer[mesh_index_offset + (3 * j) + 0] = mesh_vertex_offset + assimp_mesh->mFaces[j].mIndices[0];
			index_buffer[mesh_index_offset + (3 * j) + 1] = mesh_vertex_offset + assimp_mesh->mFaces[j].mIndices[1];
			index_buffer[mesh_index_offset + (3 * j) + 2] = mesh_vertex_offset + assimp_mesh->mFaces[j].mIndices[2];
		}
		mesh->submeshIndexCounts[i] = 3 * assimp_mesh->mNumFaces;

		mesh_vertex_offset += assimp_mesh->mNumVertices;
		mesh_index_offset += mesh->submeshIndexCounts[i];

		void *staging_memory;
		GPU_Buffer staging_buffer = Renderer::CreateGPUStagingBuffer(vertices_size + indices_size, &staging_memory);
		CopyMemory(vertex_buffer, staging_memory, vertices_size);
		CopyMemory(index_buffer, (char *)staging_memory + vertices_size, indices_size);
		mesh->gpuMesh = QueueIndexedGeometryUploadToGPU(vertices_size, indices_size, staging_buffer, uploadCounter);


		//mesh->load_status = ASSET_LOADED;
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

void FinalizeAssetUploadsToGPU(Render_API_Context *api_context) {
	// Submit queued asset upload commands.
	SwitchAssetUploadDoubleBuffer(&assetsContext.uploadCommandBuffers);
	if (assetsContext.uploadCommandBuffers.readBufferElementCount > 0) {
		Render_API_Submit_Command_Buffers(assetsContext.uploadCommandBuffers.readBufferElementCount, assetsContext.uploadCommandBuffers.readBuffer->commandBuffers, GPU_GRAPHICS_COMMAND_QUEUE, assetsContext.uploadFences[assetsContext.uploadFenceCount]);
		assetsContext.pendingUploadCounterCounts[assetsContext.uploadFenceCount] = assetsContext.uploadCommandBuffers.readBufferElementCount;
		CopyMemory(assetsContext.uploadCommandBuffers.readBuffer->uploadCounters, &assetsContext.pendingUploadCountersPerFence[assetsContext.uploadFenceCount], assetsContext.uploadCommandBuffers.readBufferElementCount * sizeof(AssetUploadCounter *));
		assetsContext.uploadFenceCount += 1;
		// @TODO: Render_API_Free_Command_Buffers(assetsContext.gpu_upload_command_pool, assetsContext.uploadCommandBuffers.readBufferElementCount, assetsContext.uploadCommandBuffers.read_buffer->command_buffers);
	}

	// Check whether submitted asset upload commands have finished.
	for (s32 i = 0; i < assetsContext.uploadFenceCount; i++) {
		if (!Render_API_Was_Fence_Signalled(assetsContext.uploadFences[i])) {
			continue;
		}
		Render_API_Reset_Fences(1, &assetsContext.uploadFences[i]);
		for (s32 j = 0; j < assetsContext.pendingUploadCounterCounts[i]; j++) {
			assetsContext.pendingUploadCountersPerFence[i][j]->commandBufferCount -= 1;
			if (assetsContext.pendingUploadCountersPerFence[i][j]->commandBufferCount == 0) {
				*assetsContext.pendingUploadCountersPerFence[i][j]->loadStatus = ASSET_LOADED;
			}
		}
		// Remove the fence.
		assetsContext.uploadFenceCount -= 1;
		if (i != assetsContext.uploadFenceCount) {
			GPU_Fence temporary = assetsContext.uploadFences[i];
			assetsContext.uploadFences[i] = assetsContext.uploadFences[assetsContext.uploadFenceCount];
			assetsContext.uploadFences[assetsContext.uploadFenceCount] = temporary;
			i -= 1;
		}
	}
}

void InitializeAssets(void *job_parameter)
{
	assetsContext.gpu_upload_command_pool = RenderAPICreateCommandPool(GPU_GRAPHICS_COMMAND_QUEUE);

	for (s32 i = 0; i < MAX_UPLOAD_FENCES; i++)
	{
		assetsContext.uploadFences[i] = RenderAPICreateFence(false);
	}
}
