#version 420

//layout(location = 0) in vec4 fragment_color;

layout(location = 0) out vec4 output_color;

layout(push_constant) uniform PER_OBJECT {
	V4 fragment_color;
} push_constants;

void main() {
	output_color = push_constants.fragment_color;
}
