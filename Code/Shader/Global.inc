#include "../Engine/ShaderGlobal.h"

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference: enable
#extension GL_EXT_scalar_block_layout: enable

//const uint meshDrawIndex = gl_BaseInstanceID;
//const uint materialDrawIndex = gl_BaseInstanceID + gl_DrawID;

struct Vertex
{
	vec3 position;
	vec3 normal;
};

layout (buffer_reference, buffer_reference_align = 8, scalar) buffer VertexBufferPointer
{
	Vertex v[];
};

layout (buffer_reference, buffer_reference_align = 8, scalar) buffer IndexBufferPointer
{
	uint i[];
};

layout (buffer_reference, std430) buffer MeshPointer
{
	layout(row_major) mat4 modelViewProjection;
};

layout (buffer_reference, std430) buffer MaterialPointer
{
	vec4 color;
};

layout (buffer_reference, std430) buffer DrawDataPointer
{
	VertexBufferPointer vertexBuffer;
	IndexBufferPointer indexBuffer;
	MeshPointer mesh;
	//MaterialPointer material;
};

layout (push_constant) uniform PushConstants
{
	DrawDataPointer drawData;
};
