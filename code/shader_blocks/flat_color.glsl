@VERTEX
OUTPUT: {
	vec4 fragment_color;
}

MAIN: {
	fragment_color = vec4(1.0, 0.0, 0.0, 1.0);
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
