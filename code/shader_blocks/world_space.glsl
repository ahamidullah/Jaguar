@VERTEX
UNIFORMS: {
	mat4 model_to_world_space; @PER_OBJECT
}

INPUT: {
	vec3 vertex_position;
}

MAIN: {
	gl_Position = model_to_world_space * vec4(vertex_position, 1.0);
}
