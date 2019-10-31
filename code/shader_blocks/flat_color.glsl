@VERTEX
UNIFORMS {
	vec4 color;
}

INPUT: {
	vec3 vertex_position;
}

MAIN: {
	fragment_color = color;
	gl_Position = ubo.model_view_projection * vec4(vertex_position, 1.0);
}

@FRAGMENT
INPUT: {
	vec4 fragment_color;
}

OUTPUT: {
	vec4 output_color;
}

MAIN: {
	output_color = fragment_color;
}
