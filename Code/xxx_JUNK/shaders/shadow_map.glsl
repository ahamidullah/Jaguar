#version 420

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 vertex_position;

layout (binding = 0) uniform UBO {
	layout(row_major) mat4 world_to_clip_space;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;   
};

void main() {
	gl_Position = ubo.world_to_clip_space * vec4(vertex_position, 1.0);
}



#else

void main() {
}

#endif
