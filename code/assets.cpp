//#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define DEFAULT_DIFFUSE_COLOR V3{1.0f, 0.08f, 0.58f}
#define DEFAULT_SPECULAR_COLOR V3{1.0f, 1.0f, 1.0f}

#define INVALID_ASSET_ID ((Asset_ID)-1)

Asset_ID get_texture(const char *path) {
	s32 texture_width, texture_height, texture_channels;
	auto pixels = stbi_load(path, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
	assert(pixels);

	load_vulkan_texture(pixels, texture_width, texture_height);

	free(pixels);

	return (Asset_ID)0;
}

// @TODO: WRITE A BLENDER EXPORTER!!!!!!!!!!!!!!!!!!
void load_model(const char *path, Asset_ID id, Game_Assets *assets, Memory_Arena *thread_local_arena) {
	const aiScene* assimp_scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
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

	auto model = (Model_Asset *)malloc(sizeof(Model_Asset));//allocate_struct(&assets->arena, Model_Asset); // @TODO: Threading. Lock asset memory arena?
	model->mesh_count = mesh_count;
	model->meshes = (Mesh_Asset *)malloc(sizeof(Mesh_Asset) * model->mesh_count);//allocate_array(&assets->arena, Mesh_Asset, model->mesh_count);
	auto vertices = (Vertex *)malloc(sizeof(Vertex) * vertex_count);//allocate_array(&assets->arena, Vertex, vertex_count);
	auto indices = (u32 *)malloc(sizeof(u32) * index_count);//allocate_array(&assets->arena, u32, index_count);

	assets->lookup[id] = model;

	auto model_directory = get_directory(path, thread_local_arena);

	u32 vertex_offset = 0;
	u32 index_offset = 0;

	for (s32 i = 0; i < mesh_count; i++) {
		auto assimp_mesh = assimp_scene->mMeshes[i];
		assert(assimp_mesh->HasPositions() && assimp_mesh->HasNormals() && assimp_mesh->HasFaces());

		model->meshes[i].vertex_count = assimp_mesh->mNumVertices;
		model->meshes[i].vertices = &vertices[vertex_offset];

		for (s32 j = 0; j < model->meshes[i].vertex_count; j++) {
			auto v = &model->meshes[i].vertices[j];

			v->position[0] = assimp_mesh->mVertices[j].x;
			v->position[1] = assimp_mesh->mVertices[j].y;
			v->position[2] = assimp_mesh->mVertices[j].z;

			v->normal[0] = assimp_mesh->mNormals[j].x;
			v->normal[1] = assimp_mesh->mNormals[j].y;
			v->normal[2] = assimp_mesh->mNormals[j].z;

			v->color[0] = 1.0f;
			v->color[1] = 0.0f;
			v->color[2] = 0.0f;
			/*
			v->color[0] = assimp_mesh->mColors[j]->r;
			v->color[1] = assimp_mesh->mColors[j]->g;
			v->color[2] = assimp_mesh->mColors[j]->b;
			*/

			if (assimp_mesh->mTextureCoords[0]) {
				v->uv[0] = assimp_mesh->mTextureCoords[0][j].x;
				v->uv[1] = assimp_mesh->mTextureCoords[0][j].y;
			} else {
				v->uv[0] = -1;
				v->uv[1] = -1;
			}
		}

		vertex_offset += model->meshes[i].vertex_count;

		model->meshes[i].index_count = 3 * assimp_mesh->mNumFaces;
		model->meshes[i].indices = &indices[index_offset];

		for (s32 j = 0; j < assimp_mesh->mNumFaces; j++) {
			auto assimp_face = assimp_mesh->mFaces[j];

			assert(assimp_face.mNumIndices == 3);

			model->meshes[i].indices[(3 * j) + 0] = assimp_face.mIndices[0];
			model->meshes[i].indices[(3 * j) + 1] = assimp_face.mIndices[1];
			model->meshes[i].indices[(3 * j) + 2] = assimp_face.mIndices[2];
		}

		aiString material_name;
		aiMaterial* assimp_material = assimp_scene->mMaterials[assimp_mesh->mMaterialIndex];
		assert(assimp_material->Get(AI_MATKEY_NAME, material_name) == aiReturn_SUCCESS);
		s32 material_id = -1;

		for (s32 j = 0; j < assets->material_count; j++) {
			if (assets->materials[j].name == material_name) {
				material_id = j;
				break;
			}
		}

		if (material_id == -1) {
			// New material.
			assets->material_count += 1;
			assert(assets->material_count < MAX_MATERIAL_COUNT);
			material_id = assets->material_count;

			auto material = &assets->materials[material_id];
			material->name = material_name;

			aiString diffuse_path, normal_path, specular_path;
			if (assimp_material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_path) == aiReturn_SUCCESS) {
				material->shader = TEXTURED_STATIC_SHADER;
				auto texture_path = join_paths(model_directory, diffuse_path.C_Str(), thread_local_arena);
				material->diffuse_map = get_texture(texture_path);
			} else {
				material->shader = UNTEXTURED_STATIC_SHADER;
				material->diffuse_map = INVALID_ASSET_ID;
			}

			if (assimp_material->GetTexture(aiTextureType_SPECULAR, 0, &specular_path) == aiReturn_SUCCESS) {
				material->specular_map = get_texture(specular_path.C_Str());
			} else {
				material->specular_map = INVALID_ASSET_ID;
			}
			
			if (assimp_material->GetTexture(aiTextureType_NORMALS, 0, &normal_path) == aiReturn_SUCCESS) {
				material->normal_map = get_texture(normal_path.C_Str());
			} else {
				material->normal_map = INVALID_ASSET_ID;
			}

			aiColor3D diffuse_color, specular_color;
			if (assimp_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color) == aiReturn_SUCCESS) {
				material->diffuse_color = {diffuse_color.r, diffuse_color.g, diffuse_color.b};
			} else {
				material->diffuse_color = DEFAULT_DIFFUSE_COLOR;
			}
			if (assimp_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color) == aiReturn_SUCCESS) {
				material->diffuse_color = {diffuse_color.r, diffuse_color.g, diffuse_color.b};
			} else {
				material->diffuse_color = DEFAULT_SPECULAR_COLOR;
			}
		}
	}

	model->vertex_gpu_memory_offset = transfer_model_data_to_gpu(model);
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
	auto asset = (Model_Asset *)game_state->assets.lookup[id];
	auto instance = &game_state->model_instances[game_state->model_instance_count];
	instance->transform = transform;
	instance->mesh_count = asset->mesh_count;
	instance->material_ids = NULL; // @TODO
	instance->vertex_gpu_memory_offset = 0; // @TODO
	instance->first_index = asset->meshes[0].indices[0]; // @TODO
	instance->index_counts = (u32 *)malloc(sizeof(u32) * asset->mesh_count);
	for (s32 i = 0; i < asset->mesh_count; i++) {
		instance->index_counts[i] = asset->meshes[i].index_count;
	}
	//instance->uniform_offset = game_state->model_instance_count * sizeof(Dynamic_Scene_UBO); // @TODO

	game_state->model_instance_count += 1;
}

void initialize_assets(Game_State *game_state) {
	const char *path = "data/male2.fbx";
	load_model(path, GUY1_ASSET, &game_state->assets, &game_state->frame_arena);
	load_model(path, GUY2_ASSET, &game_state->assets, &game_state->frame_arena);
	Transform t = {};
	create_model_instance(game_state, GUY1_ASSET, t);
	t.translation = {10.0f, 0.0, 0.0f};
	create_model_instance(game_state, GUY2_ASSET, t);
}
