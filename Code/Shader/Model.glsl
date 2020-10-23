#version 460

#include "Global.inc"

Stage: Vertex
{
	//layout (location = 0) in vec3 vertexPosition;
	//layout (location = 1) in vec3 vertexNormal;

	layout (location = 0) out vec3 fragmentNormal;
	//layout (location = 1) out flat int fragmentDrawID;

	void main()
	{
		DrawData dd = drawDataArray;
		//uint i = dd.indexBuffer.i[gl_VertexIndex];
		VertexData v = dd.vertexBuffer.v[gl_VertexIndex];
		/*
		vec4 p;
		if (gl_VertexIndex == 0)
		{
			p = dd.p1;
		}
		if (gl_VertexIndex == 1)
		{
			p = dd.p2;
		}
		if (gl_VertexIndex == 2)
		{
			p = dd.p3;
		}
		gl_Position = p;
		*/
		//gl_Position = vec4(v.position, 1.0);
		mat4 mvp = dd.meshUniforms.modelViewProjection;
		//vec3 pp = v.position;
			//gl_Position = drawDataArray.p;
			//fragmentNormal = vec3(drawDataArray.p);
		gl_Position = mvp * vec4(v.position, 1.0);
		//if (materials[0].shadingModel == PhongShadingModel)
		if (true)
		{
			//fragmentNormal = v.normal;
		}
	}
}

Stage: Fragment
{
	layout (location = 0) in vec3 fragmentNormal;

	layout (location = 0) out vec4 outputColor;

	//vec3 color = vec3(materials[materialIndices[materialDrawIndex]].color);
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
		outputColor = vec4(1.0, 0.0, 0.0, 1.0);
	/*
		//if (materials[0].shadingModel == PhongShadingModel)
		if (true)
		{
			vec3 ambientIntensity = vec3(0.15f, 0.15f, 0.15f);
			vec3 diffuseIntensity = vec3(0.6f, 0.6f, 0.6f);
			//vec3 directionalLightDirection = vec3(-0.707107, -0.707107, -0.707107);
			vec3 directionalLightDirection = normalize(vec3(0.0, 0.0, 0.0) - vec3(0.0, 1.0, 1.0));
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
	*/
	}
}
