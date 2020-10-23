#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_shader_explicit_arithmetic_types: enable 

//const uint meshDrawIndex = gl_BaseInstanceID;
//const uint materialDrawIndex = gl_BaseInstanceID + gl_DrawID;

struct VertexData
{
	vec3 position;
	vec3 normal;
};

layout (buffer_reference, buffer_reference_align = 16, scalar) buffer VertexBuffer
{
	VertexData v[];
};

/*
layout (buffer_reference, buffer_reference_align = 8, scalar) buffer IndexBuffer
{
	uint i[];
};
*/

layout (buffer_reference, scalar) buffer MeshUniforms
{
	layout(row_major) mat4 modelViewProjection;
};

layout (buffer_reference, scalar) buffer MaterialUniforms
{
	vec4 color;
};

layout (buffer_reference, buffer_reference_align = 16, std430) buffer DrawData
{
	vec4 p1;
	vec4 p2;
	vec4 p3;
	VertexBuffer vertexBuffer;
	//IndexBuffer indexBuffer;
	MeshUniforms meshUniforms;
	//MaterialPointer material;
};

layout (push_constant) uniform PushConstants
{
	DrawData drawDataArray;
};
