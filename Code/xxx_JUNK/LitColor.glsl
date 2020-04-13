#version 420

#if defined(VERTEX_SHADER)

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;

layout (location = 0) out vec3 fragmentNormal;

layout(std140, set = 0, binding = 0) uniform View
{
	layout(row_major) mat4 modelViewProjection[];
} views[];

void main()
{
	gl_Position = views[0].modelViewProjection[0] * vec4(vertexPosition, 1.0);
	fragmentNormal = vertexNormal;
}


#elif defined(FRAGMENT_SHADER)

layout(location = 0) out vec4 outputColor;

layout(location = 0) in vec3 fragmentNormal;

vec3 color = vec3(1.0, 0.0, 0.0);
vec3 ambientIntensity = vec3(0.15f, 0.15f, 0.15f);
vec3 diffuseIntensity = vec3(0.6f, 0.6f, 0.6f);
vec3 directionalLightDirection = vec3(-0.707107, -0.707107, -0.707107);

vec3 ambient(vec3 intensity)
{
	return intensity * color;
}

vec3 diffuse(vec3 intensity, vec3 receiveDirection)
{
	float impact = max(dot(fragmentNormal, receiveDirection), 0.0f);
	return intensity * impact * color;
}

void main()
{
	vec3 directionalLightingResult = vec3(0);
	{
		vec3 receiveDirection = normalize(-directionalLightDirection);
		directionalLightingResult = ambient(ambientIntensity) + diffuse(diffuseIntensity, receiveDirection);
	}

	outputColor = vec4(directionalLightingResult, 1.0);
}

#endif
