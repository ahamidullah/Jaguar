@VERTEX
UNIFORMS: {
	vec4 color; @PER_MATERIAL
	texture2D texture; @PER_MATERIAL
	texture2D texture2; @PER_OBJECT
	texture2D texture3; @PER_OBJECT
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
