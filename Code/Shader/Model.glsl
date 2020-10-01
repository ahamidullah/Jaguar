#version 420

#include "../Engine/ShaderGlobal.h"
#include "Global.inc"

Stage: Vertex
{
	layout (location = 0) in vec3 vertexPosition;
	layout (location = 1) in vec3 vertexNormal;

	layout (location = 0) out vec3 fragmentNormal;

	void main()
	{
		gl_Position = objects[0].modelViewProjection * vec4(vertexPosition, 1.0);
		if (materials[0].shadingModel == PhongShadingModel)
		{
			fragmentNormal = vertexNormal;
		}
	}
}

Stage: Fragment
{
	layout (location = 0) in vec3 fragmentNormal;

	layout (location = 0) out vec4 outputColor;

	vec3 color = vec3(1.0, 0.0, 0.0);

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
		if (materials[0].shadingModel == PhongShadingModel)
		{
			vec3 ambientIntensity = vec3(0.15f, 0.15f, 0.15f);
			vec3 diffuseIntensity = vec3(0.6f, 0.6f, 0.6f);
			vec3 directionalLightDirection = vec3(-0.707107, -0.707107, -0.707107);

			vec3 directionalLightingResult = vec3(0);
			{
				vec3 receiveDirection = normalize(-directionalLightDirection);
				directionalLightingResult = ambient(ambientIntensity) + diffuse(diffuseIntensity, receiveDirection);
			}

			outputColor = vec4(directionalLightingResult, 1.0);
		}
		else
		{
			outputColor = vec4(1.0, 0.0, 0.0, 1.0);
		}
	}
}
