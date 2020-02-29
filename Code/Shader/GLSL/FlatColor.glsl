#version 420

#if defined(VERTEX_SHADER)

layout (location = 0) in vec3 vertexPosition;

layout(std140, set = 0, binding = 0) uniform View
{
	layout(row_major) mat4 modelViewProjection[];
} views[];

void main()
{
	gl_Position = views[0].modelViewProjection[0] * vec4(vertexPosition, 1.0);
}


#elif defined(FRAGMENT_SHADER)

layout(location = 0) out vec4 outputColor;

void main()
{
	outputColor = vec4(1.0, 0.0, 0.0, 1.0);
}

#endif
