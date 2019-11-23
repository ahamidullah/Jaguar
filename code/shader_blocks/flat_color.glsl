@VERTEX
UNIFORMS: {
	vec4 color; @PER_MATERIAL
}

OUTPUT: {
	vec4 fragment_color;
}

MAIN: {
	fragment_color = color;
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
