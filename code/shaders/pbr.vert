#version 420

layout(binding = 0, set = 1) uniform uniform_buffer_object {
	vec3 camera_position;
} ubo;
layout(binding = 1, set = 3) uniform Dynamic_UBO {
	layout(row_major) mat4 model_to_world_space;
	layout(row_major) mat4 model_to_clip_space;
	layout(row_major) mat4 model_to_shadow_map_clip_space;
} dynamic_ubo;

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_color; // @TODO: Get rid of vertex_color.
layout (location = 2) in vec2 vertex_uv;
layout (location = 3) in vec3 vertex_normal;
layout (location = 4) in vec3 vertex_tangent;
layout (location = 5) in uint material_id;

layout(location = 0) out vec3 world_space_position;
layout(location = 1) out vec4 shadow_map_fragment_clip_space_position;
layout(location = 2) out vec2 fragment_uv;
//layout(location = 3) out vec3 fragment_normal;
layout(location = 3) out vec3 light_direction;
layout(location = 4) out vec3 fragment_normal;
layout(location = 5) flat out uint fragment_material_id;
layout(location = 6) out vec3 camera_position;

vec3 directional_light_direction = vec3(-0.707107, -0.707107, -0.707107);

void main() {
	vec3 T = vertex_tangent;
	vec3 N = vertex_normal;
	vec3 B = cross(N, T);
	mat3 TBN = transpose(mat3(T, B, N));
	light_direction = TBN * directional_light_direction;//vec3(dynamic_ubo.model_to_world_space * vec4(vertex_position, 0.0));
	//light_direction = directional_light_direction;//vec3(dynamic_ubo.model_to_world_space * vec4(vertex_position, 0.0));

	fragment_material_id = material_id;
	camera_position = TBN * ubo.camera_position;
	world_space_position = TBN * vec3(dynamic_ubo.model_to_world_space * vec4(vertex_position, 0.0));
	//lll = directional_light_direction;
	fragment_normal = vertex_normal;
	fragment_uv = vertex_uv;
	shadow_map_fragment_clip_space_position = dynamic_ubo.model_to_shadow_map_clip_space * vec4(vertex_position, 1.0);
	gl_Position = dynamic_ubo.model_to_clip_space * vec4(vertex_position, 1.0);
}
