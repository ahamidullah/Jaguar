#pragma once

#if DevelopmentBuild

#include "Basic/Array.h"

enum GLTFAttributeType
{
	GLTFPositionType,
	GLTFNormalType,
	GLTFTangentType,
};

struct GLTFAttribute
{
	GLTFAttributeType type;
	s64 index;
};

enum GLTFPrimitiveMode
{
	GLTFPointMode = 0,
	GLTFLineMode = 1,
	GLTFLineLoopMode = 2,
	GLTFLineStripMode = 3,
	GLTFTriangleMode = 4,
	GLTFTriangleStripMode = 5,
	GLTFTriangleFanMode = 6,
};

struct GLTFPrimitive
{
	Array<GLTFAttribute> attributes;
	s64 indices;
	GLTFPrimitiveMode mode;
	s64 material;
};

struct GLTFMesh
{
	Array<GLTFPrimitive> primitives;
	String name;
};

enum GLTFAccessorComponentType
{
	GLTFByteType = 5120,
	GLTFUnsignedByteType = 5121,
	GLTFShortType = 5122,
	GLTFUnsignedShortType = 5123,
	GLTFUnsignedIntType = 5125,
	GLTFFloatType = 5126,
};

enum GLTFAccessorType
{
	GLTFScalarType = 1,
	GLTFVec2Type = 2,
	GLTFVec3Type = 3,
	GLTFVec4Type = 4,
	GLTFMat2Type = 4,
	GLTFMat3Type = 9,
	GLTFMat4Type = 16,
};

// An accessor defines how to retreive data from a bufferView as a typed array.
struct GLTFAccessor
{
	s64 bufferView;
	s64 byteOffset;
	GLTFAccessorComponentType componentType;
	s64 count;
	GLTFAccessorType type;
};

struct GLTFMaterialPBRMetallicRoughness
{
	StaticArray<f32, 4> baseColorFactor;
	f32 metallicFactor;
};

struct GLTFMaterial
{
	GLTFMaterialPBRMetallicRoughness pbrMetallicRoughness;
	String name;
};

enum GLTFBufferViewTarget
{
	GLTFArrayBufferTarget = 34962,
	GLTFElementArrayBufferTarget = 34963,
};

struct GLTFBufferView
{
	s64 buffer;
	s64 byteOffset;
	s64 byteLength;
	s64 byteStride;
	GLTFBufferViewTarget target;
};

struct GLTFBuffer
{
	s64 byteLength;
	String uri;
	//File uriFile;
};

struct GLTF
{
	Array<GLTFMesh> meshes;
	Array<GLTFAccessor> accessors;
	Array<GLTFMaterial> materials;
	Array<GLTFBufferView> bufferViews;
	Array<GLTFBuffer> buffers;
};

GLTF GLTFParseFile(String path, bool *err);
s64 GLTFComponentTypeToSize(GLTFAccessorComponentType t);

#endif
