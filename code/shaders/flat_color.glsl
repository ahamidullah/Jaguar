#version 420

#ifdef VERTEX_SHADER

layout(binding = 0, set = 0) uniform uniform_buffer_object {
	vec4 color;
	layout(row_major) mat4 model_view_projection;
} ubo;

layout (location = 0) in vec3 vertex_position;

//layout (location = 0) out vec4 fragment_color;

void main() {
	//fragment_color = ubo.color;
	gl_Position = ubo.model_view_projection * vec4(vertex_position, 1.0);
}




#else

//layout(location = 0) in vec4 fragment_color;
layout(push_constant) uniform PER_OBJECT {
	vec4 fragment_color;
} push_constants;

layout(location = 0) out vec4 output_color;

void main() {
	output_color = push_constants.fragment_color;
}

#endif
