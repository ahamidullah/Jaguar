#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// @TODO: Trying to load the same asset multiple times simultaneously?

//#define DEFAULT_DIFFUSE_COLOR (V3){1.0f, 0.08f, 0.58f}
#define DEFAULT_DIFFUSE_COLOR (V3){0.0f, 1.00f, 0.00f}
#define DEFAULT_SPECULAR_COLOR (V3){1.0f, 1.0f, 1.0f}

GPU_Indexed_Geometry Queue_Indexed_Geometry_Upload_To_GPU(Render_Context *context, Game_Assets *assets, u32 vertices_size, u32 indices_size, GPU_Staging_Buffer staging_buffer, Asset_GPU_Upload_Counter *asset_gpu_upload_counter) {
	GPU_Buffer vertex_buffer = Create_GPU_Device_Buffer(context, vertices_size, GPU_VERTEX_BUFFER | GPU_TRANSFER_DESTINATION_BUFFER);
	GPU_Buffer index_buffer = Create_GPU_Device_Buffer(context, indices_size, GPU_INDEX_BUFFER | GPU_TRANSFER_DESTINATION_BUFFER);
	GPU_Command_Buffer command_buffer = Render_API_Create_Command_Buffer(&context->api_context, context->thread_local_contexts[thread_index].upload_command_pool);
	Render_API_Record_Copy_Buffer_Command(&context->api_context, command_buffer, vertices_size, staging_buffer.buffer, vertex_buffer, staging_buffer.offset, 0);
	Render_API_Record_Copy_Buffer_Command(&context->api_context, command_buffer, indices_size, staging_buffer.buffer, index_buffer, staging_buffer.offset + vertices_size, 0);
	Render_API_End_Command_Buffer(&context->api_context, command_buffer);
	Atomic_Write_To_Double_Buffer(assets->gpu_upload_command_buffers, command_buffer, asset_gpu_upload_counter);
	return (GPU_Indexed_Geometry){
		.vertex_buffer = vertex_buffer,
		.index_buffer = index_buffer,
	};
}

GPU_Texture_ID Queue_Texture_Upload_To_GPU(Render_Context *context, Game_Assets *assets, u8 *pixels, s32 texture_width, s32 texture_height, Asset_GPU_Upload_Counter *asset_gpu_upload_counter) {
	// @TODO: Load texture directly into staging memory.
	void *staging_memory;
	u32 texture_byte_size = sizeof(u32) * texture_width * texture_height;
	GPU_Staging_Buffer staging_buffer = Create_GPU_Staging_Buffer(context, texture_byte_size, &staging_memory);
	Copy_Memory(pixels, staging_memory, texture_byte_size);
	GPU_Image image = Create_GPU_Device_Image(context, texture_width, texture_height, GPU_FORMAT_R8G8B8A8_UNORM, GPU_IMAGE_LAYOUT_UNDEFINED, GPU_IMAGE_USAGE_TRANSFER_DST | GPU_IMAGE_USAGE_SAMPLED, GPU_SAMPLE_COUNT_1);
	GPU_Command_Buffer command_buffer = Render_API_Create_Command_Buffer(&context->api_context, context->thread_local_contexts[thread_index].upload_command_pool);
	Render_API_Transition_Image_Layout(&context->api_context, command_buffer, image, GPU_FORMAT_R8G8B8A8_UNORM, GPU_IMAGE_LAYOUT_UNDEFINED, GPU_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Render_API_Record_Copy_Buffer_To_Image_Command(&context->api_context, command_buffer, staging_buffer.buffer, image, texture_width, texture_height);
	Render_API_Transition_Image_Layout(&context->api_context, command_buffer, image, GPU_FORMAT_R8G8B8A8_UNORM, GPU_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, GPU_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	Render_API_End_Command_Buffer(&context->api_context, command_buffer);
	Atomic_Write_To_Double_Buffer(assets->gpu_upload_command_buffers, command_buffer, asset_gpu_upload_counter);
	return 0;
}

typedef struct Load_Texture_Job_Parameter {
	String path;
	Render_Context *render_context;
	Game_Assets *assets;
	Asset_GPU_Upload_Counter *asset_gpu_upload_counter;
	Texture_ID *output_texture_id;
} Load_Texture_Job_Parameter;

void Load_Texture(void *job_parameter_pointer) {
	Load_Texture_Job_Parameter *job_parameter = (Load_Texture_Job_Parameter *)job_parameter_pointer;
	s32 texture_width, texture_height, texture_channels;
	u8 *pixels = stbi_load(job_parameter->path.data, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
	assert(pixels);
	printf("%s\n", job_parameter->path.data);
	*job_parameter->output_texture_id = Queue_Texture_Upload_To_GPU(job_parameter->render_context, job_parameter->assets, pixels, texture_width, texture_height, job_parameter->asset_gpu_upload_counter);
	free(pixels);
}

// @TODO @PREPROCESSOR: Generate these.
struct {
	Asset_ID asset_id;
	const char *filepath;
} asset_id_to_filepath_map[] = {
	{ANVIL_ASSET, "data/models/anvil"},
	{GUY_ASSET, "data/models/guy"},
};

typedef struct Load_Model_Job_Parameter {
	Asset_ID asset_id;
	Render_Context *render_context;
	Game_Assets *assets;
	GPU_Upload_Flags gpu_upload_flags;
	Memory_Arena arena;
	void **output_mesh_asset_address;
} Load_Model_Job_Parameter;

void Load_Model(void *job_parameter_pointer) {
	Load_Model_Job_Parameter *job_parameter = (Load_Model_Job_Parameter *)job_parameter_pointer;
	String model_directory = {};
	for (u32 i = 0; i < Array_Count(asset_id_to_filepath_map); i++) {
		if (asset_id_to_filepath_map[i].asset_id == job_parameter->asset_id) {
			model_directory = S(asset_id_to_filepath_map[i].filepath);
			break;
		}
	}
	Assert(model_directory.data);

	String model_name = Get_Filename_From_Path(model_directory, &job_parameter->arena);

	String fbx_filename = Join_Strings(model_name, S(".fbx"));
	String fbx_filepath = Join_Filepaths(model_directory, fbx_filename, &job_parameter->arena);
	const struct aiScene* assimp_scene = aiImportFile(fbx_filepath.data, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_RemoveRedundantMaterials);
	if (!assimp_scene || assimp_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimp_scene->mRootNode) {
		Abort("assimp error: %s", aiGetErrorString());
	}

	s32 num_joints = 0;
	for (s32 i = 0; i < assimp_scene->mNumMeshes; i++) {
		num_joints += assimp_scene->mMeshes[i]->mNumBones;
	}

	u32 mesh_count = assimp_scene->mNumMeshes;

	u32 vertex_count = 0;
	u32 index_count = 0;
	for (s32 i = 0; i < mesh_count; i++) {
		vertex_count += assimp_scene->mMeshes[i]->mNumVertices;
		index_count += assimp_scene->mMeshes[i]->mNumFaces * 3;
	}

	Mesh_Asset *mesh = malloc(sizeof(Mesh_Asset));
	mesh->vertex_count = vertex_count;
	//mesh->vertices = malloc(sizeof(Vertex) * mesh->vertex_count);
	mesh->index_count = index_count;
	//mesh->indices = malloc(sizeof(u32) * mesh->index_count);
	mesh->submesh_count = mesh_count;
	mesh->submesh_index_counts = malloc(sizeof(u32) * mesh->submesh_count);; // @TODO
	mesh->materials = malloc(sizeof(Material) * mesh->submesh_count); // @TODO
	//job_parameter->assets->lookup[asset_id] = mesh;
	*(job_parameter->output_mesh_asset_address) = mesh;

	u32 mesh_vertex_offset = 0;
	u32 mesh_index_offset = 0;
	u32 max_x_vertex = 0, max_y_vertex = 0, max_z_vertex = 0;
	u32 min_x_vertex = 0, min_y_vertex = 0, min_z_vertex = 0;
	for (u32 i = 0; i < mesh_count; i++) {
		struct aiMesh *assimp_mesh = assimp_scene->mMeshes[i];
		assert(assimp_mesh->mVertices && assimp_mesh->mNormals && (assimp_mesh->mFaces && assimp_mesh->mNumFaces > 0));

		u32 vertices_size = sizeof(Vertex) * assimp_mesh->mNumVertices;
		u32 indices_size = sizeof(u32) * 3 * assimp_mesh->mNumFaces;
		Vertex *vertex_buffer = malloc(vertices_size); // @TODO
		u32 *index_buffer = malloc(indices_size); // @TODO

		for (s32 j = 0; j < assimp_mesh->mNumVertices; j++) {
			Vertex *v = &vertex_buffer[mesh_vertex_offset + j];
			v->position = (V3){
				.x = assimp_mesh->mVertices[j].x,
				.y = assimp_mesh->mVertices[j].y,
				.z = assimp_mesh->mVertices[j].z,
			};
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
		print_v3(max);
		print_v3(min);
		print_v3(bounding_box_center);
		V3 bounding_box_max_corner = {
			assimp_mesh->mVertices[max_x_vertex].x,
			assimp_mesh->mVertices[max_y_vertex].y,
			assimp_mesh->mVertices[max_z_vertex].z,
		};
		mesh->bounding_sphere = (Bounding_Sphere){
			.center = bounding_box_center,
			.radius = vector_length(subtract_v3(bounding_box_max_corner, bounding_box_center)),
		};
		print_v3(mesh->bounding_sphere.center);
		print_f32(mesh->bounding_sphere.radius);

		for (s32 j = 0; j < assimp_mesh->mNumFaces; j++) {
			assert(assimp_mesh->mFaces[j].mNumIndices == 3);
			index_buffer[mesh_index_offset + (3 * j) + 0] = mesh_vertex_offset + assimp_mesh->mFaces[j].mIndices[0];
			index_buffer[mesh_index_offset + (3 * j) + 1] = mesh_vertex_offset + assimp_mesh->mFaces[j].mIndices[1];
			index_buffer[mesh_index_offset + (3 * j) + 2] = mesh_vertex_offset + assimp_mesh->mFaces[j].mIndices[2];
		}
		mesh->submesh_index_counts[i] = 3 * assimp_mesh->mNumFaces;

		mesh_vertex_offset += assimp_mesh->mNumVertices;
		mesh_index_offset += mesh->submesh_index_counts[i];

		Asset_GPU_Upload_Counter *asset_gpu_upload_counter = malloc(sizeof(Asset_GPU_Upload_Counter));
		asset_gpu_upload_counter->load_status = &mesh->load_status;
		asset_gpu_upload_counter->gpu_command_buffer_count = 6; // @TODO

		V3 vs[3] = {
			{-1.0f, -1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{1.0f, -1.0f, 0.0f},
		};
		u32 is[3] = {
			0, 1, 2,
		};
		void *staging_memory;
		//GPU_Staging_Buffer staging_buffer = Create_GPU_Staging_Buffer(job_parameter->render_context, vertices_size + indices_size, &staging_memory);
		//Copy_Memory(vertex_buffer, staging_memory, vertices_size);
		//Copy_Memory(index_buffer, (char *)staging_memory + vertices_size, indices_size);
		//mesh->gpu_mesh = Queue_Indexed_Geometry_Upload_To_GPU(job_parameter->render_context, job_parameter->assets, vertices_size, indices_size, staging_buffer, asset_gpu_upload_counter);
		GPU_Staging_Buffer staging_buffer = Create_GPU_Staging_Buffer(job_parameter->render_context, sizeof(vs) + sizeof(is), &staging_memory);
		Copy_Memory(vs, staging_memory, sizeof(vs));
		Copy_Memory(is, (char *)staging_memory + sizeof(vs), sizeof(is));
		mesh->gpu_mesh = Queue_Indexed_Geometry_Upload_To_GPU(job_parameter->render_context, job_parameter->assets, sizeof(vs), sizeof(is), staging_buffer, asset_gpu_upload_counter);

		// Load textures.
		{
			Material *material = &mesh->materials[i];
			Load_Texture_Job_Parameter load_texture_job_parameters[] = {
				{
					.path = Join_Filepaths(model_directory, S("albedo.png"), &job_parameter->arena),
					.render_context = job_parameter->render_context,
					.assets = job_parameter->assets,
					.output_texture_id = &mesh->materials[i].albedo_map,
					.asset_gpu_upload_counter = asset_gpu_upload_counter,
				},
				{
					.path = Join_Filepaths(model_directory, S("normal.png"), &job_parameter->arena),
					.render_context = job_parameter->render_context,
					.assets = job_parameter->assets,
					.output_texture_id = &mesh->materials[i].normal_map,
					.asset_gpu_upload_counter = asset_gpu_upload_counter,
				},
				{
					.path = Join_Filepaths(model_directory, S("roughness.png"), &job_parameter->arena),
					.render_context = job_parameter->render_context,
					.assets = job_parameter->assets,
					.output_texture_id = &mesh->materials[i].roughness_map,
					.asset_gpu_upload_counter = asset_gpu_upload_counter,
				},
				{
					.path = Join_Filepaths(model_directory, S("metallic.png"), &job_parameter->arena),
					.render_context = job_parameter->render_context,
					.assets = job_parameter->assets,
					.output_texture_id = &mesh->materials[i].metallic_map,
					.asset_gpu_upload_counter = asset_gpu_upload_counter,
				},
				{
					.path = Join_Filepaths(model_directory, S("ambient_occlusion.png"), &job_parameter->arena),
					.render_context = job_parameter->render_context,
					.assets = job_parameter->assets,
					.output_texture_id = &mesh->materials[i].ambient_occlusion_map,
					.asset_gpu_upload_counter = asset_gpu_upload_counter,
				},
			};
			Job_Declaration load_texture_job_declarations[Array_Count(load_texture_job_parameters)];
			for (s32 j = 0; j < Array_Count(load_texture_job_parameters); j++) {
				load_texture_job_declarations[j] = Create_Job(Load_Texture, &load_texture_job_parameters[j]);
			}
			Job_Counter counter;
			Run_Jobs(Array_Count(load_texture_job_declarations), load_texture_job_declarations, NORMAL_JOB_PRIORITY, &counter);
			Wait_For_Job_Counter(&counter); // @TODO: Why wait????
		}

		//mesh->load_status = ASSET_LOADED;
#if 0
			struct aiString diffuse_path;
			if (aiGetMaterialTexture(assimp_material, aiTextureType_DIFFUSE, 0, &diffuse_path, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS) {
				material->shader = TEXTURED_STATIC_SHADER;
				const char *texture_path = join_paths(model_directory, diffuse_path.data, thread_local_arena);
				material->diffuse_map = load_texture(texture_path, assets);
			} else {
				material->shader = UNTEXTURED_STATIC_SHADER;
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

// @TODO
void *Acquire_Memory(size_t size, u64 tag) {
	return malloc(size);
}

void Release_Memory(void *memory) {
	free(memory);
}

// @TODO: Shouldn't we call Get_Model_Asset?
// @TODO: Just return a regular pointer!
Mesh_Asset **Get_Mesh_Asset(Asset_ID asset_id, Game_Assets *assets, Render_Context *render_context, GPU_Upload_Flags gpu_upload_flags, Job_Counter *job_counter) {
	Clear_Job_Counter(job_counter);
	if (assets->lookup[asset_id]) {
		return assets->lookup[asset_id];
	}
	Load_Model_Job_Parameter *job_parameter = Acquire_Memory(sizeof(Load_Model_Job_Parameter), 0); // @TODO: Release memory.
	job_parameter->asset_id = asset_id;
	job_parameter->arena = make_memory_arena();
	job_parameter->render_context = render_context;
	job_parameter->assets = assets;
	job_parameter->gpu_upload_flags = gpu_upload_flags;
	job_parameter->output_mesh_asset_address = &assets->lookup[asset_id];
	Job_Declaration job_declaration = Create_Job(Load_Model, job_parameter);
	Run_Jobs(1, &job_declaration, NORMAL_JOB_PRIORITY, job_counter);
	return (Mesh_Asset **)&assets->lookup[asset_id];
}

void Finalize_Asset_GPU_Uploads(Game_Assets *assets, Render_API_Context *api_context) {
	// Submit queued asset upload commands.
	//Console_Print("read_buffer_element_count: %d\n", assets->gpu_upload_command_buffers.read_buffer_element_count);
	Switch_Double_Buffer(assets->gpu_upload_command_buffers);
	if (assets->gpu_upload_command_buffers.read_buffer_element_count > 0) {
		GPU_Fence fence = Render_API_Submit_Command_Buffers(api_context, assets->gpu_upload_command_buffers.read_buffer_element_count, assets->gpu_upload_command_buffers.read_buffer->command_buffers, GPU_GRAPHICS_COMMAND_QUEUE);
		assets->gpu_upload_fences[assets->gpu_upload_fence_count] = fence;
		assets->pending_gpu_upload_counter_counts[assets->gpu_upload_fence_count] = assets->gpu_upload_command_buffers.read_buffer_element_count;
		Copy_Memory(assets->gpu_upload_command_buffers.read_buffer->gpu_upload_counters, &assets->pending_gpu_upload_counters[assets->gpu_upload_fence_count], assets->gpu_upload_command_buffers.read_buffer_element_count * sizeof(Asset_GPU_Upload_Counter *));
		assets->gpu_upload_fence_count += 1;
		// @TODO: Render_API_Free_Command_Buffers(api_context, assets->gpu_upload_command_pool, assets->gpu_upload_command_buffers.read_buffer_element_count, assets->gpu_upload_command_buffers.read_buffer->command_buffers);
	}

	// Check whether submitted asset upload commands have finished.
	// @TODO: Destroy fences?
	//Console_Print("gpu_upload_fence_count: %d\n", assets->gpu_upload_fence_count);
	for (s32 i = 0; i < assets->gpu_upload_fence_count; i++) {
		if (Render_API_Was_Fence_Signalled(api_context, assets->gpu_upload_fences[i])) {
			for (s32 j = 0; j < assets->pending_gpu_upload_counter_counts[i]; j++) {
				//Console_Print("gpu_command_buffer_count: %d\n", assets->pending_gpu_upload_counters[i][j]->gpu_command_buffer_count);
				assets->pending_gpu_upload_counters[i][j]->gpu_command_buffer_count -= 1;
				if (assets->pending_gpu_upload_counters[i][j]->gpu_command_buffer_count == 0) {
					*assets->pending_gpu_upload_counters[i][j]->load_status = ASSET_LOADED;
				}
			}

			// Remove the fence.
			assets->gpu_upload_fence_count -= 1;
			if (i != assets->gpu_upload_fence_count) {
				assets->gpu_upload_fences[i] = assets->gpu_upload_fences[assets->gpu_upload_fence_count];
				i -= 1;
			}
		}
	}
}

void Initialize_Assets(void *job_parameter) {
	Game_State *game_state = (Game_State *)job_parameter;
	game_state->assets.gpu_upload_command_pool = Render_API_Create_Command_Pool(&game_state->render_context.api_context, GPU_GRAPHICS_COMMAND_QUEUE);
	Create_Double_Buffer(game_state->assets.gpu_upload_command_buffers);
	//load_model("data/models/anvil", NANOSUIT_ASSET, &game_state->assets, &game_state->frame_arena); // @TODO @SUBARENA
}
