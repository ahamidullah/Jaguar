#version 420

#if defined(VERTEX_SHADER)

layout uniform UBO {
	vec4 color;
	mat4 modelViewProjection;
} ubo;

layout in vec3 vertexPosition;
layout out vec4 fragmentColor;

void main() {
	fragmentColor = ubo.color;
	gl_Position = ubo.modelViewProjection * vec4(vertexPosition, 1.0);
}



#elif defined(FRAGMENT_SHADER)

layout in vec4 fragmentColor;
layout out vec4 outputColor;

void main() {
	outputColor = fragmentColor;
}

#endif
