#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//#define DEFAULT_DIFFUSE_COLOR (V3){1.0f, 0.08f, 0.58f}
#define DEFAULT_DIFFUSE_COLOR (V3){0.0f, 1.00f, 0.00f}
#define DEFAULT_SPECULAR_COLOR (V3){1.0f, 1.0f, 1.0f}

#define INVALID_ASSET_ID ((Asset_ID)-1)

Texture_ID load_texture(const char *path, Game_Assets *assets) {
	s32 texture_width, texture_height, texture_channels;
	printf("%s\n", path);
	u8 *pixels = stbi_load(path, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
	assert(pixels);
	Texture_ID id = load_vulkan_texture(pixels, texture_width, texture_height);
	free(pixels);
	return id;
}

// @TODO: WRITE A BLENDER EXPORTER!!!!!!!!!!!!!!!!!!
void load_model(const char *path, Asset_ID id, Game_Assets *assets, Memory_Arena *thread_local_arena) {
	const char *model_directory = get_directory(path, thread_local_arena);

	const struct aiScene* assimp_scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_RemoveRedundantMaterials);
	if (!assimp_scene || assimp_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimp_scene->mRootNode) {
		_abort("assimp error: %s", aiGetErrorString());
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

	Loaded_Mesh *mesh = malloc(sizeof(Loaded_Mesh));
	mesh->vertex_count = vertex_count;
	mesh->vertices = malloc(sizeof(Vertex) * mesh->vertex_count);
	mesh->index_count = index_count;
	mesh->indices = malloc(sizeof(u32) * mesh->index_count);
	mesh->submesh_count = mesh_count;
	mesh->submesh_index_counts = malloc(sizeof(u32) * mesh->submesh_count);;
	mesh->materials = malloc(sizeof(Material) * mesh->submesh_count);
	assets->lookup[id] = mesh;

	u32 mesh_vertex_offset = 0;
	u32 mesh_index_offset = 0;
	for (s32 i = 0; i < mesh_count; i++) {
		struct aiMesh *assimp_mesh = assimp_scene->mMeshes[i];
		assert(assimp_mesh->mVertices && assimp_mesh->mNormals && (assimp_mesh->mFaces && assimp_mesh->mNumFaces > 0));

		//model->meshes[i].vertex_count = assimp_mesh->mNumVertices;
		//model->meshes[i].vertices = &vertices[mesh_vertex_offset];

		for (s32 j = 0; j < assimp_mesh->mNumVertices; j++) {
			Vertex *v = &mesh->vertices[mesh_vertex_offset + j];

			v->position.x = assimp_mesh->mVertices[j].x;
			v->position.y = assimp_mesh->mVertices[j].y;
			v->position.z = assimp_mesh->mVertices[j].z;

			v->normal.x = assimp_mesh->mNormals[j].x;
			v->normal.y = assimp_mesh->mNormals[j].y;
			v->normal.z = assimp_mesh->mNormals[j].z;

			v->color.x = 1.0f;
			v->color.y = 0.0f;
			v->color.z = 0.0f;
			/*
			v->color[0] = assimp_mesh->mColors[j]->r;
			v->color[1] = assimp_mesh->mColors[j]->g;
			v->color[2] = assimp_mesh->mColors[j]->b;
			*/

			if (assimp_mesh->mTextureCoords[0]) {
				v->uv.x = assimp_mesh->mTextureCoords[0][j].x;
				v->uv.y = assimp_mesh->mTextureCoords[0][j].y;
			} else {
				v->uv.x = -1;
				v->uv.y = -1;
			}
		}

		for (s32 j = 0; j < assimp_mesh->mNumFaces; j++) {
			assert(assimp_mesh->mFaces[j].mNumIndices == 3);
			mesh->indices[mesh_index_offset + (3 * j) + 0] = mesh_vertex_offset + assimp_mesh->mFaces[j].mIndices[0];
			mesh->indices[mesh_index_offset + (3 * j) + 1] = mesh_vertex_offset + assimp_mesh->mFaces[j].mIndices[1];
			mesh->indices[mesh_index_offset + (3 * j) + 2] = mesh_vertex_offset + assimp_mesh->mFaces[j].mIndices[2];
		}
		mesh->submesh_index_counts[i] = 3 * assimp_mesh->mNumFaces;

		mesh_vertex_offset += assimp_mesh->mNumVertices;
		mesh_index_offset += mesh->submesh_index_counts[i];

		struct aiMaterial* assimp_material = assimp_scene->mMaterials[assimp_mesh->mMaterialIndex];
		s32 material_id = -1;

		// @TODO: Some way to share materials between different meshes.

		if (material_id == -1) {
			// New material.
			//material_id = assets->material_count++;
			//assert(assets->material_count < MAX_MATERIAL_COUNT);

			Material *material = &mesh->materials[i];
			//material->name = material_name;

			struct aiString diffuse_path, normal_path, specular_path;
			if (aiGetMaterialTexture(assimp_material, aiTextureType_DIFFUSE, 0, &diffuse_path, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS) {
				material->shader = TEXTURED_STATIC_SHADER;
				const char *texture_path = join_paths(model_directory, diffuse_path.data, thread_local_arena);
				material->diffuse_map = load_texture(texture_path, assets);
			} else {
				material->shader = UNTEXTURED_STATIC_SHADER;
				material->diffuse_map = INVALID_ASSET_ID;
			}

			if (aiGetMaterialTexture(assimp_material, aiTextureType_SPECULAR, 0, &specular_path, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS) {
				const char *texture_path = join_paths(model_directory, specular_path.data, thread_local_arena);
				//material->specular_map = load_texture(texture_path, assets);
			} else {
				material->specular_map = INVALID_ASSET_ID;
			}
			
			if (aiGetMaterialTexture(assimp_material, aiTextureType_NORMALS, 0, &normal_path, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS) {
				const char *texture_path = join_paths(model_directory, normal_path.data, thread_local_arena);
				//material->normal_map = load_texture(texture_path, assets);
			} else {
				material->normal_map = INVALID_ASSET_ID;
			}

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
		}
	}

	transfer_model_data_to_gpu(mesh, &mesh->vertex_offset, &mesh->first_index);
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

void create_model_instance(Game_State *game_state, Asset_ID id, Transform transform) {
/*
	Model_Asset *asset = game_state->assets.lookup[id];
	Model_Instance *instance = &game_state->model_instances[game_state->model_instance_count];
	instance->transform = transform;
	instance->mesh_count = asset->mesh_count;
	//instance->material_ids = NULL; // @TODO
	//instance->vertex_gpu_memory_offset = asset->vertex_gpu_memory_offset; // @TODO
	instance->vertex_offset = asset->vertex_offset;
	instance->first_index = asset->first_index; // @TODO WRONG
	instance->index_counts = malloc(sizeof(u32) * asset->mesh_count);
	for (s32 i = 0; i < asset->mesh_count; i++) {
		instance->index_counts[i] = asset->mesh_index_counts[i];
	}
	game_state->model_instance_count += 1;
*/
}

void initialize_assets(Game_State *game_state) {
	const char *path = "data/male2.fbx";
	load_model(path, GUY1_ASSET, &game_state->assets, &game_state->frame_arena);
	//load_model("data/models/gun/Handgun_Packed.fbx", GUN_ASSET, &game_state->assets, &game_state->frame_arena);
	load_model("data/models/nanosuit/nanosuit.obj", NANOSUIT_ASSET, &game_state->assets, &game_state->frame_arena);
	//load_model(path, GUY2_ASSET, &game_state->assets, &game_state->frame_arena);
	//load_model(path, GUY3_ASSET, &game_state->assets, &game_state->frame_arena);
	/*
	Transform t = {};
	create_model_instance(game_state, GUY1_ASSET, t);
	t.translation = (V3){0.0f, -2.0, 0.0f};
	//create_model_instance(game_state, GUY2_ASSET, t);
	t.translation = (V3){0.0f, 10.0, 0.0f};
	create_model_instance(game_state, NANOSUIT_ASSET, t);
	*/
}
