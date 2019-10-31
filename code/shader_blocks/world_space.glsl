@VERTEX
UNIFORM: {
	mat4 model_view_projection;
}

INPUT: {
	vec3 vertex_position;
}

MAIN: {
	gl_Position = model_view_projection * vec4(vertex_position, 1.0);
}
