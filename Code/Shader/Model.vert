#version 420

#include "../Engine/ShaderGlobal.h"
#include "Global.inc"

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;

layout (location = 0) out vec3 fragmentNormal;

void main()
{
	gl_Position = objects[0].modelViewProjection * vec4(vertexPosition, 1.0);
	if (materials[0].shadingModel == PHONG_SHADING_MODEL)
	{
		fragmentNormal = vertexNormal;
	}
}
