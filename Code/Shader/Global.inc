layout (std140, set = ShaderGlobalDescriptorSetIndex, binding = 0) uniform Global
{
	uint dummy;
} global;

layout (std140, set = ShaderViewDescriptorSetIndex, binding = 0) uniform View
{
	uint dummy;
} views[];

layout (std140, set = ShaderMaterialDescriptorSetIndex, binding = 0) uniform Material
{
	uint shadingModel;
} materials[];

layout (std140, set = ShaderObjectDescriptorSetIndex, binding = 0) uniform Object
{
	layout(row_major) mat4 modelViewProjection;
} objects[];
