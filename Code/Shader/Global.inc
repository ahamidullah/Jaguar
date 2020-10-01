layout (std140, set = ShaderGlobalDescriptorSet, binding = 0) uniform Global
{
	uint dummy;
} global;

layout (std140, set = ShaderViewDescriptorSet, binding = 0) uniform View
{
	uint dummy;
} views[];

layout (std140, set = ShaderMaterialDescriptorSet, binding = 0) uniform Material
{
	uint shadingModel;
} materials[];

layout (std140, set = ShaderObjectDescriptorSet, binding = 0) uniform Object
{
	layout(row_major) mat4 modelViewProjection;
} objects[];
