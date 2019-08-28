#version 420

layout(binding = 0, set = 0) uniform uniform_buffer_object {
	vec4 color;
	layout(row_major) mat4 model_view_projection;
} ubo;

layout (location = 0) in vec3 vertex_position;

layout (location = 0) out vec4 fragment_color;

void main() {
	fragment_color = ubo.color;
	gl_Position = ubo.model_view_projection * vec4(vertex_position, 1.0);
}
