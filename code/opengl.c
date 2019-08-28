#define DEFINEPROC
#include "opengl_functions.h"
#undef DEFINEPROC

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define MAX_POINT_GRAPHICS 512

#include <vector>

typedef u32 Asset_ID;

struct Mesh_ {
	u32 vao;
	u32 vbo;
	u32 ebo;
	u32 index_count;
	u32 texture_id;
};

struct Model_ {
	u32 num_meshes;
	Static_Array<Mesh_> meshes;
};

//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.
//@TODO: Change lookup to a hash table.

struct Models_ {
	Array<u32> lookup;
	Array<Model_> instances;
};

struct Skeleton_Joint_Pose {
	Quaternion rotation;
	V3 translation;
	f32 scale;
};

struct Skeleton_Skinning_Info_ {
	u32 num_vertex_influences;
	Static_Array<u32> vertices;
	Static_Array<f32> weights;
};

// @TODO: We could store animation transforms as 4x3 matrix, maybe?
struct Skeleton_Asset {
	u8 num_joints;
	Static_Array<aiString> names; // @TODO: Get rid of this aiString.
	Static_Array<Skeleton_Skinning_Info_> skinning_info;
	Static_Array<u8> parent_indices;
	Static_Array<M4> inverse_rest_pose;

	u8 num_leaf_nodes;
	Static_Array<u8> leaf_node_parent_indices;
	Static_Array<M4> leaf_node_translations; // @TODO @Memory: Could probably just be a V3 translation?
};

struct Skeleton_Instance {
	Skeleton_Asset *asset;

	Static_Array<Skeleton_Joint_Pose> local_joint_poses; // Currently, this is parent * node_transform, but it should probably just be node_transform for blending purposes.
	Static_Array<M4> global_joint_poses; // Currently, this is parent * node_transform, but it should probably just be node_transform for blending purposes.
};

struct Skeletons_ {
	Array<u32> lookup;
	Array<Skeleton_Asset> assets;
	Array<Skeleton_Instance> instances;
};

struct Animation_Sample {
	Array<Skeleton_Joint_Pose> joint_poses;
};

struct Animation_Asset {
	Asset_ID skeleton_id;
	Static_Array<Animation_Sample> samples;
	f32 num_frames;
	f32 frames_per_second;
	s8 looped;
};

struct Animation_Instance {
	Animation_ *info;

	f32 time;
	u32 current_frame;
};

struct Animations {
	Array<u32> lookup;
	Array<Animation_Asset> assets;
	Array<Animation_Instance> instances;
};

M4 world_perspective_projection;

V4 black = {0.0f, 0.0f, 0.0f, 1.0f};
V4 white = {1.0f, 1.0f, 1.0f, 1.0f};
V4 blue = {0.0f, 0.0f, 1.0f, 1.0f};
V4 red = {1.0f, 0.0f, 0.0f, 1.0f};
V4 green = {0.0f, 1.0f, 0.0f, 1.0f};
V4 purple = {1.0f, 0.0f, 1.0f, 1.0f};

typedef void (*OpenGL_Error_Function)(u32, GLenum, GLint *);
typedef void (*OpenGL_Info_Function)(u32, GLsizei, GLsizei *, GLchar *);

char *check_opengl_error(u32 obj, OpenGL_Error_Function ivfn, u32 objparam, OpenGL_Info_Function infofn) {
	GLint status;
	ivfn(obj, objparam, &status);
	if (status == GL_FALSE) {
		GLint infolog_len;
		ivfn(obj, GL_INFO_LOG_LENGTH, &infolog_len);
		GLchar *infolog = (GLchar *)malloc(infolog_len + 1);
		infofn(obj, infolog_len, NULL, infolog);
		return infolog;
	}
	return NULL;
}

u32 compile_shader(const GLenum type, char *source, const char *defines = "") {
	assert(source);

	auto id = glCreateShader(type);

	const char *source_strings[] = { "#version 330\n", defines, source };
	glShaderSource(id, ARRAY_COUNT(source_strings), source_strings, NULL);
	glCompileShader(id);

	auto error = check_opengl_error(id, glGetShaderiv, GL_COMPILE_STATUS, glGetShaderInfoLog);
	if (error) {
		_abort("Error compiling shader:\n"
		       "%s\n"
		       "In shader:\n"
		       "%s\n",
		       error, source);
	}

	return id;
}

u32 build_shader(const char *vert_file_path, const char *frag_file_path) {
	assert(vert_file_path);
	assert(frag_file_path);

	auto vert_source = read_entire_file(vert_file_path);
	if (vert_source.error) {
		_abort("Failed to read shader file %s:\n%s", vert_file_path, vert_source.error);
	}

	auto frag_source = read_entire_file(frag_file_path);
	if (frag_source.error) {
		_abort("Failed to read shader file %s:\n%s", frag_file_path, frag_source.error);
	}

	DEFER(free(vert_source.contents));
	DEFER(free(frag_source.contents));

	auto vert_id = compile_shader(GL_VERTEX_SHADER, vert_source.contents);
	auto frag_id = compile_shader(GL_FRAGMENT_SHADER, frag_source.contents);

	auto program = glCreateProgram();
	glAttachShader(program, vert_id);
	glAttachShader(program, frag_id);
	glLinkProgram(program);

	auto error = check_opengl_error(program, glGetProgramiv, GL_LINK_STATUS, glGetProgramInfoLog);
	if (error) {
		_abort("Error linking shader:\n"
		       "%s\n"
		       "In vertex shader:\n"
		       "%s\n"
		       "In fragment shader:\n"
		       "%s\n",
		       error, vert_source, frag_source);
	}

	glDetachShader(program, vert_id);
	glDetachShader(program, frag_id);
	glDeleteShader(vert_id);
	glDeleteShader(frag_id);

	return program;
}

u32 animated_mesh_shader, shadow_map_shader, text_shader, rgba_no_texture_shader;

u32 load_texture(const char *path) {
	u32 texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    s32 width, height, channel_count;
    u8 *image_data = stbi_load(path, &width, &height, &channel_count, 0);
    DEFER(stbi_image_free(image_data));

	if (!image_data) {
		_abort("Could not find texture at %s", path);
	}

	if (channel_count == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	} else if (channel_count == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
	} else {
		_abort("invalid image channel count %d\n", channel_count);
	}
	glGenerateMipmap(GL_TEXTURE_2D);

	return texture_id;
}

struct Vertex {
	f32 position[3];
	f32 normal[3];
	f32 uv[2]; // Switch to short?
};

struct Skinned_Vertex {
	f32 position[3];
	f32 normal[3];
	f32 uv[2]; // @TODO @Memory: Switch to short?
	s32 joint_indices[4]; // @TODO @Memory: Switch to short?
	f32 weights[4]; // @TODO @Memory: Switch to a smaller float?
	// @TODO @Memory: Should we omit the last weight vertex? Can be calculated at runtime.
};

struct Mesh {
	u32 vao;
	u32 vbo;
	u32 ebo;
	u32 index_count;
	u32 texture_id;
};

struct Skeleton_Skinning_Info {
	std::vector<u32> vertices;
	std::vector<f32> weights;
};

struct Skeleton_Info {
	std::vector<aiString> names;

	std::vector<Skeleton_Skinning_Info> skinning_info;
	std::vector<s32> parent_indices;
	std::vector<M4> joint_to_mesh_transform;

	std::vector<s32> leaf_node_parent_indices;
	std::vector<M4> leaf_node_transforms; // @TODO: Could probably just be a V3 translation?
};

struct Skeleton {
	Skeleton_Info *info;

	std::vector<M4> joint_transforms; // Currently, this is parent * node_transform, but it should probably just be node_transform for blending purposes.
};

struct Skeleton_Joint_Transform {
	V3 scale;
	V3 translation;
	Quaternion rotation;
};

struct Animation_Key_Frame {
	std::vector<Skeleton_Joint_Transform> joint_transforms;

	f32 time = 0.0f;
};

struct Animation_Info {
	std::vector<Animation_Key_Frame> key_frames;

	f32 time = 0;
	u32 current_frame = 0;
	f32 duration = 0;
	f32 seconds_per_frame = 0;
};

struct Animation {
	Animation_Info *info;

	f32 time = 0;
	u32 current_frame = 0;
};

struct Animation_Player {
	Animation_Info *info;

	f32 time = 0;
	u32 current_frame = 0;
};

struct Model {
	std::vector<Mesh> meshes;
	Skeleton skeleton;
	Animation animation;
};

M4 assimp_matrix_to_m4(aiMatrix4x4 *m) {
	return M4{
		m->a1, m->a2, m->a3, m->a4,
		m->b1, m->b2, m->b3, m->b4,
		m->c1, m->c2, m->c3, m->c4,
		m->d1, m->d2, m->d3, m->d4,
	};
}

#if 0
void process_assimp_node(Model *model, aiNode* assimp_node, const aiScene* assimp_scene, char *fbx_directory) {
	for (s32 i = 0; i < assimp_node->mNumMeshes; i++) {
		aiMesh* assimp_mesh = assimp_scene->mMeshes[assimp_node->mMeshes[i]];

		// Assimp keeps track of indices per mesh.

		assert(assimp_mesh->HasPositions() && assimp_mesh->HasNormals() && assimp_mesh->HasFaces());

		Mesh mesh;

		glGenVertexArrays(1, &mesh.vao);
		glBindVertexArray(mesh.vao);

		size_t vertex_buffer_size = sizeof(Vertex) * assimp_mesh->mNumVertices;
		Vertex *vertex_buffer = (Vertex *)emalloc(vertex_buffer_size);
		DEFER(free(vertex_buffer));

		for (s32 j = 0; j < assimp_mesh->mNumVertices; j++) {
			Vertex *v = &vertex_buffer[j];

			v->position[0] = assimp_mesh->mVertices[j].x;
			v->position[1] = assimp_mesh->mVertices[j].y;
			v->position[2] = assimp_mesh->mVertices[j].z;

			v->bone_indices[0] = 0;
			v->bone_indices[1] = 0;
			v->bone_indices[2] = 0;
			v->bone_indices[3] = 0;

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
			for (s32 k = 0; k < assimp_mesh->mBones[j]->mNumWeights; k++) {
				u32 id = assimp_mesh->mBones[j]->mWeights[k].mVertexId;
				for (s32 l = 0; l < 4; l++) {
					if (vertex_buffer[id].weights[l] == 0) {
						vertex_buffer[id].bone_indices[l] = j;
						vertex_buffer[id].weights[l] = assimp_mesh->mBones[j]->mWeights[k].mWeight;
						break;
					}
				}
			}
		}

//#if 0
		Skeleton_Info *skeleton_info = new Skeleton_Info;
		skeleton_info->parent_indices.resize(assimp_mesh->mNumBones);
		skeleton_info->skinning_info.resize(assimp_mesh->mNumBones);
		skeleton_info->joint_to_mesh_transform.resize(assimp_mesh->mNumBones);
		skeleton_info->names.resize(assimp_mesh->mNumBones);

		skeleton_info->parent_indices[0] = UINT8_MAX;

		model->skeleton.joint_transforms.resize(assimp_mesh->mNumBones);

		for (s32 j = 0; j < assimp_mesh->mNumBones; j++) {
			skeleton_info->names[j] = assimp_mesh->mBones[j]->mName;
			skeleton_info->skinning_info[j].vertices.resize(assimp_mesh->mBones[j]->mNumWeights);
			skeleton_info->skinning_info[j].weights.resize(assimp_mesh->mBones[j]->mNumWeights);

			for (s32 k = 0; k < assimp_mesh->mBones[j]->mNumWeights; k++) {
				skeleton_info->skinning_info[j].vertices[k] = assimp_mesh->mBones[j]->mWeights[k].mVertexId;
				skeleton_info->skinning_info[j].weights[k] = assimp_mesh->mBones[j]->mWeights[k].mWeight;

				u32 id = skeleton_info->skinning_info[j].vertices[k];
				for (s32 l = 0; l < 4; l++) {
					if (vertex_buffer[id].weights[l] == 0) {
						vertex_buffer[id].bone_indices[l] = j;
						vertex_buffer[id].weights[l] = skeleton_info->skinning_info[j].weights[k];
						break;
					}
				}

				aiMatrix4x4 *m = &assimp_mesh->mBones[j]->mOffsetMatrix;
				skeleton_info->joint_to_mesh_transform[j] = M4{
					m->a1, m->a2, m->a3, m->a4,
					m->b1, m->b2, m->b3, m->b4,
					m->c1, m->c2, m->c3, m->c4,
					m->d1, m->d2, m->d3, m->d4,
				};
			}
		}

		auto root = assimp_scene->mRootNode;

		for (s32 j = 0; j < model->skeleton.joint_transforms.size(); j++) {
			auto n = root->FindNode(skeleton_info->names[j]);
			assert(n != NULL);

			aiMatrix4x4 *m = &n->mTransformation;
			model->skeleton.joint_transforms[j] = M4{
				m->a1, m->a2, m->a3, m->a4,
				m->b1, m->b2, m->b3, m->b4,
				m->c1, m->c2, m->c3, m->c4,
				m->d1, m->d2, m->d3, m->d4,
			};
		}

		for (s32 j = 0; j < model->skeleton.joint_transforms.size(); j++) {
			auto n = root->FindNode(skeleton_info->names[j]);
			assert(n != NULL);

			for (s32  k = 0; k < n->mNumChildren; k++) {
				auto child = n->mChildren[k];
				if (child->mName == aiString(std::string(n->mName.C_Str()) + "_end")) {
					aiMatrix4x4 *m = &child->mTransformation;
					auto transform = M4{
						m->a1, m->a2, m->a3, m->a4,
						m->b1, m->b2, m->b3, m->b4,
						m->c1, m->c2, m->c3, m->c4,
						m->d1, m->d2, m->d3, m->d4,
					};
					skeleton_info->leaf_node_parent_indices.push_back(j);
					skeleton_info->leaf_node_transforms.push_back(transform);
					break;
				}

				for (s32 l = 0; l < model->skeleton.joint_transforms.size(); l++) {
					if (skeleton_info->names[l] == child->mName) {
						skeleton_info->parent_indices[l] = j;
						model->skeleton.joint_transforms[l] = model->skeleton.joint_transforms[skeleton_info->parent_indices[l]] * model->skeleton.joint_transforms[l];
						break;
					}
				}
			}
		}

		model->skeleton.info = skeleton_info;
//#endif

		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertex_buffer, GL_DYNAMIC_DRAW);

		mesh.index_count = assimp_mesh->mNumFaces * 3;
		size_t index_buffer_size = sizeof(u32) * mesh.index_count;
		u32 *index_buffer = (u32 *)emalloc(index_buffer_size);
		DEFER(free(index_buffer));

		for (s32 j = 0; j < assimp_mesh->mNumFaces; j++) {
			aiFace assimp_face = assimp_mesh->mFaces[j];

			assert(assimp_face.mNumIndices == 3);

			for (s32 k = 0; k < assimp_face.mNumIndices; k++) {
				index_buffer[(3 * j) + k] = assimp_face.mIndices[k];
			}
		}

		glGenBuffers(1, &mesh.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, index_buffer, GL_DYNAMIC_DRAW);

		// Doesn't handle multiple textures per mesh.
		mesh.texture_id = 0;
		aiMaterial* mat = assimp_scene->mMaterials[assimp_mesh->mMaterialIndex];
		aiString diffuse_path, specular_path; // Relative to the fbx file's directory.
		aiReturn has_diffuse = mat->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_path);
		if (has_diffuse == aiReturn_SUCCESS) {
			auto texture_path = join_paths_temp(fbx_directory, diffuse_path.C_Str());
			mesh.texture_id = load_texture(texture_path);
		}

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, normal));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, uv));
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (GLvoid *)offsetof(Vertex, bone_indices));
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, weights));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

		model->meshes.push_back(mesh);
	}

	for (u32  i = 0; i < assimp_node->mNumChildren; i++) {
		process_assimp_node(model, assimp_node->mChildren[i], assimp_scene, fbx_directory);
	}
}

void load_animation(Animation *animation, aiNode *assimp_node) {
}

void load_skeleton(Skeleton *skeleton, aiNode *assimp_node, const aiScene* assimp_scene) {
	s32 num_joints = 0;
	for (s32 i = 0; i < assimp_node->mNumMeshes; i++) {
		num_joints += assimp_scene->mMeshes[assimp_node->mMeshes[i]]->mNumBones;
	}

	Skeleton_Info *skeleton_info = new Skeleton_Info;
	skeleton_info->parent_indices.resize(num_joints);
	skeleton_info->skinning_info.resize(num_joints);
	skeleton_info->joint_to_mesh_transform.resize(num_joints);
	skeleton_info->names.resize(num_joints);

	skeleton->joint_transforms.resize(num_joints);

	s32 joint_base_index = 0;

	auto root = assimp_scene->mRootNode;

	for (s32 i = 0; i < assimp_node->mNumMeshes; i++) {
		aiMesh* assimp_mesh = assimp_scene->mMeshes[assimp_node->mMeshes[i]];

		for (s32 j = 0; j < assimp_mesh->mNumBones; j++) {
			s32 joint_index = joint_base_index + j;

			skeleton_info->names[joint_index] = assimp_mesh->mBones[j]->mName;
			skeleton_info->skinning_info[joint_index].vertices.resize(assimp_mesh->mBones[j]->mNumWeights);
			skeleton_info->skinning_info[joint_index].weights.resize(assimp_mesh->mBones[j]->mNumWeights);

			for (s32 k = 0; k < assimp_mesh->mBones[j]->mNumWeights; k++) {
				skeleton_info->skinning_info[joint_index].vertices[k] = assimp_mesh->mBones[j]->mWeights[k].mVertexId;
				skeleton_info->skinning_info[joint_index].weights[k] = assimp_mesh->mBones[j]->mWeights[k].mWeight;

				u32 id = skeleton_info->skinning_info[joint_index].vertices[k];
				for (s32 l = 0; l < 4; l++) {
					if (vertex_buffer[id].weights[l] == 0) {
						vertex_buffer[id].bone_indices[l] = j;
						vertex_buffer[id].weights[l] = skeleton_info->skinning_info[joint_index].weights[k];
						break;
					}
				}

				skeleton_info->joint_to_mesh_transform[joint_index] = assimp_matrix_to_m4(&assimp_mesh->mBones[j]->mOffsetMatrix);
			}

			auto bone_node = root->FindNode(skeleton_info->names[joint_index]);
			assert(bone_node != NULL);

			skeleton->joint_transforms[joint_index] = assimp_matrix_to_m4(&bone_node->mTransformation);
			
			if (bone_node->parent == NULL) {
				skeleton_info->parent_indices[joint_index] = UINT8_MAX;
				continue;
			}

			for (s32 k = 0; k < joint_index; k++) {
				if (skeleton_info->names[k] == bone_node->parent->mName) {
					skeleton_info->parent_indices[joint_index] = k;
					skeleton->joint_transforms[joint_index] = skeleton->joint_transforms[k] * skeleton->joint_transforms[j];
					break;
				}
			}
		}

		joint_base_index += assimp_mesh->mNumBones;

//#if 0
		for (s32 j = 0; j < model->skeleton.joint_transforms.size(); j++) {
			auto n = root->FindNode(skeleton_info->names[j]);
			assert(n != NULL);

			for (s32  k = 0; k < n->mNumChildren; k++) {
				auto child = n->mChildren[k];
				if (child->mNumChildren == 0) {
					M4 transform = assimp_matrix_to_m4(&child->mTransformation);
					skeleton_info->leaf_node_parent_indices.push_back(j);
					skeleton_info->leaf_node_transforms.push_back(transform);
					break;
				}

				for (s32 l = 0; l < model->skeleton.joint_transforms.size(); l++) {
					if (skeleton_info->names[l] == child->mName) {
						skeleton_info->parent_indices[l] = j;
						model->skeleton.joint_transforms[l] = model->skeleton.joint_transforms[skeleton_info->parent_indices[l]] * model->skeleton.joint_transforms[l];
						break;
					}
				}
			}
		}
//#endif
	//}

	//model->skeleton.info = skeleton_info;
}
#endif

Animation_Info _animation;

Models_ models_;
Skeletons_ skeletons_;
Animations_ animations_;

// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!
// @TODO: Move memory to blocks/chunks!!!!!!!!!

// @TODO: Make the load_ functions only load the assets, not load and create an instance.

void load_animation(Entity_ID id, const char *path) {
	const aiScene* assimp_scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
	if (!assimp_scene || assimp_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimp_scene->mRootNode) {
		_abort("assimp error: %s", aiGetErrorString());
	}

	assert(assimp_scene->HasAnimations());

	auto aiAnimations = assimp_scene->mAnimations;

	auto animation = array_add(&animations_.assets, 1);

	// @TODO: Stop duplicating bone transform info? Would lead to less memory used, but more complicated (slower?) animation code.
	for (s32 i = 0; i < 1; i++) {
		u32 num_samples = 0;
		for (s32 j = 0; j < aiAnimations[i]->mNumChannels; j++) {
			assert(aiAnimations[i]->mChannels[j]->mNumPositionKeys == aiAnimations[i]->mChannels[j]->mNumRotationKeys);
			assert(aiAnimations[i]->mChannels[j]->mNumPositionKeys == aiAnimations[i]->mChannels[j]->mNumScalingKeys);

			if (aiAnimations[i]->mChannels[j]->mNumPositionKeys > num_key_frames) {
				num_samples = aiAnimations[i]->mChannels[j]->mNumPositionKeys;
			}
		}

		animation->frames_per_second = aiAnimations[i]->mTicksPerSecond;
		animation->skeleton_id = 0;
		animation->num_frames = aiAnimations[i]->mDuration;
		animation->looped = false;
		allocate_array(&animation->samples, num_samples);

		for (s32 j = 0; j < num_samples; j++) {
			allocate_array(&animation.samples[j].joint_poses, aiAnimations[i]->mNumChannels);
		}

		for (s32 j = 0; j < aiAnimations[i]->mNumChannels; j++) {
			u8 found_joint = false;
			u32 joint_index = 0;
			for (s32 k = 0; k < model->skeleton.joint_transforms.size(); k++) {
				if (aiAnimations[i]->mChannels[j]->mNodeName == model->skeleton.info->names[k]) {
					joint_index = k;
					found_joint = true;
					break;
				}
			}
			if (!found_joint)  continue;

			u32 key_frame_index = 0;
			for (s32 k = 0; k < aiAnimations[i]->mChannels[j]->mNumPositionKeys; k++) {
				auto pk = &aiAnimations[i]->mChannels[j]->mPositionKeys[k];
				auto rk = &aiAnimations[i]->mChannels[j]->mRotationKeys[k];
				auto sk = &aiAnimations[i]->mChannels[j]->mScalingKeys[k];

				while (key_frame_index < (u32)pk->mTime) {
					auto current_transform = &_animation.key_frames[key_frame_index].joint_transforms[joint_index];
					auto previous_transform = &_animation.key_frames[key_frame_index-1].joint_transforms[joint_index];

					current_transform->translation = previous_transform->translation;
					current_transform->rotation    = previous_transform->rotation;
					current_transform->scale       = previous_transform->scale;

					_animation.key_frames[key_frame_index].time = key_frame_index * _animation.seconds_per_frame;

					key_frame_index += 1;
				}

				_animation.key_frames[key_frame_index].time = key_frame_index * _animation.seconds_per_frame;

				auto transform = &_animation.key_frames[key_frame_index].joint_transforms[joint_index];
				transform->translation = V3{pk->mValue.x, pk->mValue.y, pk->mValue.z};
				transform->rotation    = Quaternion{rk->mValue.x, rk->mValue.y, rk->mValue.z, rk->mValue.w};
				transform->scale       = V3{sk->mValue.x, sk->mValue.y, sk->mValue.z};

				key_frame_index += 1;
			}
		}
	}
}

// @TODO: WRITE A BLENDER EXPORTER!!!!!!!!!!!!!!!!!!
void load_model(Entity_ID id, Model *model, const char *path) {
	const aiScene* assimp_scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
	if (!assimp_scene || assimp_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimp_scene->mRootNode) {
		_abort("assimp error: %s", aiGetErrorString());
	}

	s32 num_joints = 0;
	for (s32 i = 0; i < assimp_scene->mNumMeshes; i++) {
		num_joints += assimp_scene->mMeshes[i]->mNumBones;
	}

	auto model_directory = get_directory(path);

	glUseProgram(animated_mesh_shader);

	model->meshes.resize(assimp_scene->mNumMeshes);

	models_.lookup[id] = models_.instances.count;
	Model_ *model_ = array_add(&models_.instances, 1);
	allocate_array(&model_->meshes, assimp_scene->mNumMeshes);
	model_->num_meshes = assimp_scene->mNumMeshes;

	if (num_joints == 0) {
		for (s32 i = 0; i < assimp_scene->mNumMeshes; i++) {
			aiMesh* assimp_mesh = assimp_scene->mMeshes[i];
			assert(assimp_mesh->HasPositions() && assimp_mesh->HasNormals() && assimp_mesh->HasFaces());

			Mesh_ *mesh = &model_->meshes[i];
			glGenVertexArrays(1, &mesh->vao);
			glBindVertexArray(mesh->vao);

			Static_Array<Vertex> vertex_buffer;
			allocate_array(&vertex_buffer, assimp_mesh->mNumVertices);

			for (s32 j = 0; j < assimp_mesh->mNumVertices; j++) {
				Vertex *v = &vertex_buffer[j];

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

			glGenBuffers(1, &mesh->vbo);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * assimp_mesh->mNumVertices, vertex_buffer.data, GL_DYNAMIC_DRAW);
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

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, position));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, normal));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, uv));

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
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

#if 0
	if (num_joints > 0) {
		skeletons_.lookup[id] = array_count(skeletons_data);

		Skeleton_ *skeleton_ = array_add(skeletons_.data);
		skeleton_->joint_transforms = array_allocate(M4, num_joints);

		Skeleton_Asset *skeleton_asset = array_add(skeletons_.assets);
		skeleton_info->parent_indices = allocate_array(u32, num_joints);
		skeleton_info->skinning_info = allocate_array(u32, num_joints);
		skeleton_info->joint_to_mesh_transform.resize(num_joints);
		skeleton_info->names.resize(num_joints);
		skeleton_info->parent_indices[0] = UINT8_MAX;

		skeleton_->asset = skeleton_asset_;

		s32 joint_base_index = 0;

		aiNode *root = assimp_scene->mRootNode;

		for (s32 i = 0; i < assimp_scene->mNumMeshes; i++) {
			aiMesh* assimp_mesh = &assimp_scene->mMeshes[i];

			for (s32 j = 0; j < assimp_mesh->mNumBones; j++) {
				s32 joint_index = joint_base_index + j;

				skeleton_info->names[joint_index] = assimp_mesh->mBones[j]->mName;
				skeleton_info->skinning_info[joint_index].vertices = allocate_array(u32, assimp_mesh->mBones[j]->mNumWeights);
				skeleton_info->skinning_info[joint_index].weights = allocate_array(f32, assimp_mesh->mBones[j]->mNumWeights);

				for (s32 k = 0; k < assimp_mesh->mBones[j]->mNumWeights; k++) {
					skeleton_info->skinning_info[joint_index].vertices[k] = assimp_mesh->mBones[j]->mWeights[k].mVertexId;
					skeleton_info->skinning_info[joint_index].weights[k] = assimp_mesh->mBones[j]->mWeights[k].mWeight;

					u32 id = assimp_mesh->mBones[j]->mWeights[k].mVertexId;
					for (s32 l = 0; l < 4; l++) {
						if (vertex_buffer[id].weights[l] == 0) {
							vertex_buffer[id].bone_indices[l] = j;
							vertex_buffer[id].weights[l] = assimp_mesh->mBones[j]->mWeights[k].mWeight;
							break;
						}
					}
					skeleton_info->joint_to_mesh_transform[joint_index] = assimp_matrix_to_m4(&assimp_mesh->mBones[j]->mOffsetMatrix);
				}

				aiNode *bone_node = root->FindNode(skeleton_info->names[joint_index]);
				assert(bone_node != NULL);

				skeleton_->joint_transforms[joint_index] = assimp_matrix_to_m4(&bone_node->mTransformation);
				
				if (bone_node->mParent == NULL)  continue;

				for (s32 k = 0; k < joint_index; k++) {
					if (skeleton_info->names[k] == bone_node->mParent->mName) {
						skeleton_info->parent_indices[joint_index] = k;
						model->skeleton.joint_transforms[joint_index] = model->skeleton.joint_transforms[k] * model->skeleton.joint_transforms[j];
						break;
					}
				}
			}

			joint_base_index += assimp_mesh->mNumBones;
		}
	}
#endif

#if 0
	// Load skeleton.
	Skeleton_Info *skeleton_info = new Skeleton_Info;
	skeleton_info->parent_indices.resize(num_joints);
	skeleton_info->skinning_info.resize(num_joints);
	skeleton_info->joint_to_mesh_transform.resize(num_joints);
	skeleton_info->names.resize(num_joints);
	skeleton_info->parent_indices[0] = UINT8_MAX;

	model->skeleton.joint_transforms.resize(num_joints);

	s32 joint_base_index = 0;

	auto root = assimp_scene->mRootNode;

	for (s32 i = 0; i < assimp_scene->mNumMeshes; i++) {
		aiMesh* assimp_mesh = assimp_scene->mMeshes[i];

		for (s32 j = 0; j < assimp_mesh->mNumBones; j++) {
			s32 joint_index = joint_base_index + j;

			skeleton_info->names[joint_index] = assimp_mesh->mBones[j]->mName;
			skeleton_info->skinning_info[joint_index].vertices.resize(assimp_mesh->mBones[j]->mNumWeights);
			skeleton_info->skinning_info[joint_index].weights.resize(assimp_mesh->mBones[j]->mNumWeights);

			for (s32 k = 0; k < assimp_mesh->mBones[j]->mNumWeights; k++) {
				skeleton_info->skinning_info[joint_index].vertices[k] = assimp_mesh->mBones[j]->mWeights[k].mVertexId;
				skeleton_info->skinning_info[joint_index].weights[k] = assimp_mesh->mBones[j]->mWeights[k].mWeight;

				/*
				u32 id = skeleton_info->skinning_info[joint_index].vertices[k];
				for (s32 l = 0; l < 4; l++) {
					if (vertex_buffer[id].weights[l] == 0) {
						vertex_buffer[id].bone_indices[l] = j;
						vertex_buffer[id].weights[l] = skeleton_info->skinning_info[joint_index].weights[k];
						break;
					}
				}
				*/

				skeleton_info->joint_to_mesh_transform[joint_index] = assimp_matrix_to_m4(&assimp_mesh->mBones[j]->mOffsetMatrix);
			}

			auto bone_node = root->FindNode(skeleton_info->names[joint_index]);
			assert(bone_node != NULL);

			model->skeleton.joint_transforms[joint_index] = assimp_matrix_to_m4(&bone_node->mTransformation);
			
			if (bone_node->mParent == NULL)  continue;

			for (s32 k = 0; k < joint_index; k++) {
				if (skeleton_info->names[k] == bone_node->mParent->mName) {
					skeleton_info->parent_indices[joint_index] = k;
					model->skeleton.joint_transforms[joint_index] = model->skeleton.joint_transforms[k] * model->skeleton.joint_transforms[j];
					break;
				}
			}
		}

		joint_base_index += assimp_mesh->mNumBones;
	}

	model->skeleton.info = skeleton_info;

	if (assimp_scene->HasAnimations()) {
		auto aiAnimations = assimp_scene->mAnimations;

		// @TODO: Stop duplicating bone transform info? Would lead to less memory used, but more complicated (slower?) animation code.
		for (s32 i = 0; i < 1; i++) {
			_animation.seconds_per_frame = 1.0f / (f32)aiAnimations[i]->mTicksPerSecond;
			_animation.duration = aiAnimations[i]->mDuration * _animation.seconds_per_frame;

			u32 num_key_frames = 0;
			for (s32 j = 0; j < aiAnimations[i]->mNumChannels; j++) {
				assert(aiAnimations[i]->mChannels[j]->mNumPositionKeys == aiAnimations[i]->mChannels[j]->mNumRotationKeys);
				assert(aiAnimations[i]->mChannels[j]->mNumPositionKeys == aiAnimations[i]->mChannels[j]->mNumScalingKeys);

				if (aiAnimations[i]->mChannels[j]->mNumPositionKeys > num_key_frames) {
					num_key_frames = aiAnimations[i]->mChannels[j]->mNumPositionKeys;
				}
			}
			_animation.key_frames.resize(num_key_frames);

			for (s32 j = 0; j < num_key_frames; j++) {
				_animation.key_frames[j].joint_transforms.resize(model->skeleton.joint_transforms.size());
			}

			for (s32 j = 0; j < aiAnimations[i]->mNumChannels; j++) {
				u8 found_joint = false;
				u32 joint_index = 0;
				for (s32 k = 0; k < model->skeleton.joint_transforms.size(); k++) {
					if (aiAnimations[i]->mChannels[j]->mNodeName == model->skeleton.info->names[k]) {
						joint_index = k;
						found_joint = true;
						break;
					}
				}
				if (!found_joint)  continue;

				u32 key_frame_index = 0;
				for (s32 k = 0; k < aiAnimations[i]->mChannels[j]->mNumPositionKeys; k++) {
					auto pk = &aiAnimations[i]->mChannels[j]->mPositionKeys[k];
					auto rk = &aiAnimations[i]->mChannels[j]->mRotationKeys[k];
					auto sk = &aiAnimations[i]->mChannels[j]->mScalingKeys[k];

					while (key_frame_index < (u32)pk->mTime) {
						auto current_transform = &_animation.key_frames[key_frame_index].joint_transforms[joint_index];
						auto previous_transform = &_animation.key_frames[key_frame_index-1].joint_transforms[joint_index];

						current_transform->translation = previous_transform->translation;
						current_transform->rotation    = previous_transform->rotation;
						current_transform->scale       = previous_transform->scale;

						_animation.key_frames[key_frame_index].time = key_frame_index * _animation.seconds_per_frame;

						key_frame_index += 1;
					}

					_animation.key_frames[key_frame_index].time = key_frame_index * _animation.seconds_per_frame;

					auto transform = &_animation.key_frames[key_frame_index].joint_transforms[joint_index];
					transform->translation = V3{pk->mValue.x, pk->mValue.y, pk->mValue.z};
					transform->rotation    = Quaternion{rk->mValue.x, rk->mValue.y, rk->mValue.z, rk->mValue.w};
					transform->scale       = V3{sk->mValue.x, sk->mValue.y, sk->mValue.z};

					key_frame_index += 1;
				}
			}
		}
	}
#endif
}

/*
void opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	log_print(STANDARD_LOG, "source: %d, type: %d, id: %d, severity: %d, message: %s", source, type, id, severity, message);
}
*/

#define SHADOW_MAP_WIDTH 1024
#define SHADOW_MAP_HEIGHT 1024
u32 shadow_map_fbo;
u32 shadow_map_texture;

void init_fonts();

Model *guy;
std::vector<Model> models;

u32 create_entity() {
	static u32 counter = 0;
	return counter++;
}

void init_renderer() {
	#define LOADPROC
	#include "opengl_functions.h"
	#undef LOADPROC

	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
	//glDebugMessageCallback(opengl_debug_message_callback, NULL);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glViewport(0, 0, window_width, window_height);

	animated_mesh_shader = build_shader("code/animated_mesh_shader.vert", "code/animated_mesh_shader.frag");
	shadow_map_shader = build_shader("code/shadow_map_shader.vert", "code/shadow_map_shader.frag");
	text_shader = build_shader("code/text_shader.vert", "code/text_shader.frag");
	rgba_no_texture_shader = build_shader("code/rgba_no_texture_shader.vert", "code/rgba_no_texture_shader.frag");

	models.resize(10);

	allocate_array(&models_.lookup, 100);
	allocate_array(&models_.instances, 100);

	allocate_array(&skeletons_.lookup, 100);
	allocate_array(&skeletons_.assets, 100);
	allocate_array(&skeletons_.instances, 100);

	allocate_array(&animations_.lookup, 100);
	allocate_array(&animations_.assets, 100);
	allocate_array(&animations_.instances, 100);

	u32 guy_ = create_entity();
	load_model(guy_, &models[0], "data/models/source/male2.fbx");
	guy = &models[0];

	f32 aspect_ratio = (f32)window_width / (f32)window_height;
	world_perspective_projection = perspective_projection(90, aspect_ratio);

	init_fonts();

	glGenFramebuffers(1, &shadow_map_fbo);

	glGenTextures(1, &shadow_map_texture);
	glBindTexture(GL_TEXTURE_2D, shadow_map_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	f32 border_color[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

	glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(animated_mesh_shader);
	glUniform1i(glGetUniformLocation(animated_mesh_shader, "diffuse_texture"), 0);
	glUniform1i(glGetUniformLocation(animated_mesh_shader, "shadow_map_texture"), 1);
}

enum Origin {
	CENTER_ORIGIN,
	TOP_LEFT_ORIGIN,
};

void get_offset_xy(s32 *x, s32 *y, s32 width, s32 height, Origin origin) {
	switch (origin) {
	case CENTER_ORIGIN: {
		*x -= width / 2.0f;
		*y += height / 2.0f;
	} break;
	case TOP_LEFT_ORIGIN: {

	} break;
	default: {
		assert(0);
	}
	}
}

void draw_text(const char *text, Font *font, s32 x, s32 y, Origin origin, V4 color) {
	glUseProgram(text_shader);
	glDepthFunc(GL_ALWAYS);
	glUniform4f(glGetUniformLocation(text_shader, "text_color"), color.x, color.y, color.z, color.w);
	glActiveTexture(GL_TEXTURE0);

	u32 vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);

	s32 w = 0.0f, h = font->height;
	for (u8 *c = (u8 *)text; *c; c += 1) {
		Glyph_Info *gi = &font->glyph_info[*c];
		w += gi->advance;
	}

	s32 offset_x = x, offset_y = y;
	get_offset_xy(&offset_x, &offset_y, w, h, origin);

	for (u8 *c = (u8 *)text; *c; c += 1) {
		Glyph_Info *gi = &font->glyph_info[*c];

		f32 glyph_x = offset_x + gi->x_bearing;
		f32 glyph_y = offset_y - (gi->height - gi->y_bearing);
		f32 glyph_w = gi->width;
		f32 glyph_h = gi->height;

		f32 vertices[6][4] = {
			{ glyph_x,            glyph_y + glyph_h,  0.0,  0.0 },
			{ glyph_x,            glyph_y,            0.0,  1.0 },
			{ glyph_x + glyph_w,  glyph_y,            1.0,  1.0 },

			{ glyph_x,            glyph_y + glyph_h,  0.0,  0.0 },
			{ glyph_x + glyph_w,  glyph_y,            1.0,  1.0 },
			{ glyph_x + glyph_w,  glyph_y + glyph_h,  1.0,  0.0 },
		};

		glBindTexture(GL_TEXTURE_2D, gi->texture_id);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
		glDrawArrays(GL_TRIANGLES, 0, 6);

		offset_x += gi->advance;
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDepthFunc(GL_LESS);
}

void draw_shadowed_text(const char *text, Font *font, s32 x, s32 y, Origin origin, V4 fg_color, V4 bg_color) {
	draw_text(text, font, x - 1, y - 1, origin, bg_color);
	draw_text(text, font, x, y, origin, fg_color);
}

void draw_rectangle(s32 x, s32 y, s32 w, s32 h, Origin origin, V4 color) {
	s32 offset_x = x, offset_y = y;
	get_offset_xy(&offset_x, &offset_y, w, h, origin);

	u32 vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);
}

M4 light_transform;

void draw_quad(Camera *camera, V3 p, f32 w, f32 h, V3 normal, V4 color) {
	// @TODO: Move this out to a create basis function.
	V3 forward;
	if (normal == V3{0, 0, 1}) {
		forward = h * normalize(cross_product(normal, V3{0, 1, 0}));
	} else {
		forward = h * normalize(cross_product(normal, V3{0, 0, 1}));
	}
	auto side = w * normalize(cross_product(forward, normal));

	V3 bottom_left = p - forward/2 - side/2;
	V3 top_left = bottom_left + forward;
	V3 top_right = top_left + side;
	V3 bottom_right = bottom_left + side;

	V3 positions[] = {bottom_left, bottom_right, top_right, top_right, top_left, bottom_left};
	V4 colors[] = {color, color, color, color, color, color};

	glUseProgram(rgba_no_texture_shader);

	// @TODO: Move this out.
	M4 mvp_matrix = world_perspective_projection * camera->view_matrix;
	glUniformMatrix4fv(glGetUniformLocation(rgba_no_texture_shader, "render_transform"), 1, true, (f32 *)&mvp_matrix);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow_map_texture);
	glUniformMatrix4fv(glGetUniformLocation(rgba_no_texture_shader, "light_transform"), 1, true, (f32 *)&light_transform);

	u32 vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(colors), colors);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (GLvoid *)sizeof(positions));
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void draw_line(Camera *camera, V3 p1, V3 p2, V4 color, f32 width = 1.0f) {
	V3 positions[] = {p1, p2};
	V4 colors[] = {color, color};

	glLineWidth(width);
	glUseProgram(rgba_no_texture_shader);

	// @TODO: Move this out.
	M4 mvp_matrix = world_perspective_projection * camera->view_matrix;
	glUniformMatrix4fv(glGetUniformLocation(rgba_no_texture_shader, "render_transform"), 1, true, (f32 *)&mvp_matrix);

	u32 vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(colors), colors);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (GLvoid *)sizeof(positions));
	glDrawArrays(GL_LINES, 0, 2);

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void draw_skeleton(Camera *camera, Skeleton *skeleton) {
	f32 line_thickness = 2.0f;

	glDepthFunc(GL_ALWAYS);

	for (s32 i = 0; i < skeleton->joint_transforms.size(); i++) {
		s32 parent = skeleton->info->parent_indices[i];
		if (parent == UINT8_MAX) {
			continue;
		}
		auto start = get_translation(skeleton->joint_transforms[parent]);
		auto end = get_translation(skeleton->joint_transforms[i]);
		draw_line(camera, start, end, purple, line_thickness);
	}

	for (s32 i = 0; i < skeleton->info->leaf_node_transforms.size(); i++) {
		s32 parent = skeleton->info->leaf_node_parent_indices[i];
		auto start = get_translation(skeleton->joint_transforms[parent]);
		auto end = get_translation(skeleton->joint_transforms[parent] * skeleton->info->leaf_node_transforms[i]);
		draw_line(camera, start, end, purple, line_thickness);
	}

	for (s32 i = 0; i < skeleton->joint_transforms.size(); i++) {
		if (skeleton->info->parent_indices[i] == UINT8_MAX) {
			continue;
		}
		auto end = get_translation(skeleton->joint_transforms[i]);
		draw_quad(camera, end, 0.1f, 0.1f, normalize(camera->position - end), red);
	}

	glDepthFunc(GL_LESS);
}

constexpr s32 MAX_BONES = 100;
M4 bone_transform_matrices_buffer[MAX_BONES];

void draw_camera_info(Camera *camera);

void render(f32 delta_time, Camera &camera) {
	glViewport(0, 0, window_width, window_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(animated_mesh_shader);

	s32 num_bone_transform_matrices = 0;
	for (s32 i = 0; i < skeletons_.instances[0].asset->num_joints; i++) {
		bone_transform_matrices_buffer[num_bone_transform_matrices++] = skeletons_.instances[0].global_joint_poses[i] * skeletons_.instances[0].asset->inverse_rest_pose[i];
	}

	glUniformMatrix4fv(glGetUniformLocation(animated_mesh_shader, "joints"), skeletons_.instances[0].asset->num_joints, true, (f32 *)bone_transform_matrices_buffer);

	M4 render_transform = world_perspective_projection * camera.view_matrix;
	glUniformMatrix4fv(glGetUniformLocation(animated_mesh_shader, "render_transform"), 1, true, (f32 *)&render_transform);
	glUniformMatrix4fv(glGetUniformLocation(animated_mesh_shader, "light_transform"), 1, true, (f32 *)&light_transform);
	auto model_transform = m4_identity();
	glUniformMatrix4fv(glGetUniformLocation(animated_mesh_shader, "model_transform"), 1, true, (f32 *)&model_transform);

	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, shadow_map_texture);
	//glActiveTexture(GL_TEXTURE0);

	for (s32 i = 0; i < models_.instances.count; i++) {
		for (s32 j = 0; j < models_.instances[i].num_meshes; j++) {
			Mesh_ *mesh = &models_.instances[i].meshes[j];
			glUniform1i(glGetUniformLocation(animated_mesh_shader, "has_texture"), mesh->texture_id != 0);
			glBindVertexArray(mesh->vao);
			glBindTexture(GL_TEXTURE_2D, mesh->texture_id);
			glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
		}
	}

#if 0
	_animation.time += delta_time / 1000.0f;
	if (_animation.time > _animation.duration) {
		_animation.time = fmod(_animation.time, _animation.duration);
	}
	_animation.current_frame = _animation.time / _animation.seconds_per_frame;

	s32 num_bone_transform_matrices = 0;
	auto current_frame = &_animation.key_frames[_animation.current_frame];

	for (s32 i = 0; i < current_frame->joint_transforms.size(); i++) {
		V3 translation, scale;
		Quaternion rotation;

		if (_animation.key_frames.size() == 1 || _animation.current_frame == _animation.key_frames.size()) {
			translation = current_frame->joint_transforms[i].translation;
			scale       = current_frame->joint_transforms[i].scale;
			rotation    = current_frame->joint_transforms[i].rotation;
		} else {
			auto next_frame = &_animation.key_frames[_animation.current_frame + 1];
			auto percent = (_animation.time - current_frame->time) / (next_frame->time - current_frame->time);

			translation = lerp(current_frame->joint_transforms[i].translation, next_frame->joint_transforms[i].translation, percent);
			scale       = lerp(current_frame->joint_transforms[i].scale,       next_frame->joint_transforms[i].scale,       percent);
			rotation    = lerp(current_frame->joint_transforms[i].rotation,    next_frame->joint_transforms[i].rotation,    percent);
		}

		M4 *transform = &guy->skeleton.joint_transforms[i];

		set_rotation(transform, to_matrix(rotation));
		set_translation(transform, translation);
		set_scale(transform, scale);

		s32 parent = guy->skeleton.info->parent_indices[i];
		if (parent != UINT8_MAX) {
			guy->skeleton.joint_transforms[i] = guy->skeleton.joint_transforms[parent] * guy->skeleton.joint_transforms[i];
		}

		bone_transform_matrices_buffer[num_bone_transform_matrices++] = guy->skeleton.joint_transforms[i] * guy->skeleton.info->joint_to_mesh_transform[i];
	}

	glUseProgram(shadow_map_shader);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map_shader, "bones"), num_bone_transform_matrices, true, (f32 *)bone_transform_matrices_buffer);

	auto light_projection = orthographic_projection(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 40.0f);
	auto light_view = look_at(V3{3.0f, 0.0f, 3.0f}, V3{0.0f, 0.0f, 0.0f}, V3{0.0f, 0.0f, 1.0f});
	light_transform = light_projection * light_view;
	auto model_transform = m4_identity();
	glUniformMatrix4fv(glGetUniformLocation(shadow_map_shader, "light_transform"), 1, true, (f32 *)&light_transform);

	glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	for (auto &m : guy->meshes) {
		glBindVertexArray(m.vao);
		glDrawElements(GL_TRIANGLES, m.index_count, GL_UNSIGNED_INT, 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, window_width, window_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(animated_mesh_shader);
	glUniformMatrix4fv(glGetUniformLocation(animated_mesh_shader, "bones"), num_bone_transform_matrices, true, (f32 *)bone_transform_matrices_buffer);

	M4 render_transform = world_perspective_projection * camera.view_matrix;
	glUniformMatrix4fv(glGetUniformLocation(animated_mesh_shader, "render_transform"), 1, true, (f32 *)&render_transform);
	glUniformMatrix4fv(glGetUniformLocation(animated_mesh_shader, "light_transform"), 1, true, (f32 *)&light_transform);
	glUniformMatrix4fv(glGetUniformLocation(animated_mesh_shader, "model_transform"), 1, true, (f32 *)&model_transform);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadow_map_texture);
	glActiveTexture(GL_TEXTURE0);

	for (auto &m : guy->meshes) {
		glUniform1i(glGetUniformLocation(animated_mesh_shader, "has_texture"), m.texture_id != 0);
		glBindVertexArray(m.vao);
		glBindTexture(GL_TEXTURE_2D, m.texture_id);
		glDrawElements(GL_TRIANGLES, m.index_count, GL_UNSIGNED_INT, 0);
	}

	draw_quad(&camera, V3{20, -20, -6.75}, 100, 100, V3{0, 0, 1}, blue);

	draw_skeleton(&camera, &guy->skeleton);

	draw_camera_info(&camera);
#endif
}
