layout (std140, set = GLOBAL_DESCRIPTOR_SET_INDEX, binding = 0) uniform Global
{
	uint dummy;
} global;

layout (std140, set = VIEW_DESCRIPTOR_SET_INDEX, binding = 0) uniform View
{
	uint dummy;
} views[];

layout (std140, set = MATERIAL_DESCRIPTOR_SET_INDEX, binding = 0) uniform Material
{
	uint shadingModel;
} materials[];

layout (std140, set = OBJECT_DESCRIPTOR_SET_INDEX, binding = 0) uniform Object
{
	layout(row_major) mat4 modelViewProjection;
} objects[];
