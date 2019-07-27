#version 420

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 vertex_uv;
layout (location = 3) in vec3 vertex_normal;

layout(location = 0) out vec3 world_space_position;
layout(location = 1) out vec4 shadow_map_position;
layout(location = 2) out vec2 fragment_uv;
layout(location = 3) out vec3 fragment_normal;

layout(binding = 0) uniform UniformBufferObject {
	/*
	   mat4 render_transorm;
	   mat4 biased_shadow_map_transform;
	 */
	/*
	   mat4 model;
	   mat4 view;
	   mat4 projection;
	   mat4 light_model;
	   mat4 light_view;
	   mat4 light_projection;
	 */
	//layout(row_major) mat4 model_transform;
	//layout(row_major) mat4 world_to_clip_space;
	layout(row_major) mat4 world_to_shadow_map_clip_space;
} ubo;

layout(binding = 1) uniform Dynamic_UBO {
	layout(row_major) mat4 world_to_clip_space;
	layout(row_major) mat4 world_to_shadow_map_clip_space;
} dynamic_ubo;

const mat4 position_bias = mat4( 
	0.5, 0.0, 0.0, 0.5,
	0.0, 0.5, 0.0, 0.5,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
);

void main() {
	//mat4 render_transform = ubo.model_transform * ubo.view * ubo.projection;
	//mat4 light_transform = ubo.light_model * ubo.light_view * ubo.light_projection;

	fragment_normal = vertex_normal;
	fragment_uv = vertex_uv;
	//world_space_position =  vec3(ubo.model_transform * vec4(position, 1.0));
	shadow_map_position =  dynamic_ubo.world_to_shadow_map_clip_space * vec4(position, 1.0);

	//gl_Position = ubo.world_to_clip_space * vec4(position, 1.0);
	gl_Position = dynamic_ubo.world_to_clip_space * vec4(position, 1.0);
	//gl_Position = render_transform * vec4(position, 1.0);
}
