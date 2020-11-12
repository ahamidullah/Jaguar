#if DevelopmentBuild

#include "GLTF.h"
#include "Basic/JSON.h"

f32 ParseFloatAbort(String s)
{
	auto err = false;
	auto f = ParseFloat(s, &err);
	if (err)
	{
		Abort("String", "Failed to parse float %k.", s);
	}
	return f;
}

s64 ParseIntAbort(String s)
{
	auto err = false;
	auto n = ParseInteger(s, &err);
	if (err)
	{
		Abort("String", "Failed parse integer for string %k.", s);
	}
	return n;
}

GLTFBuffer ParseGLTFBuffer(parse::Parser *p)
{
	auto b = GLTFBuffer{};
	JSONParseObject(p, [&b](parse::Parser *p, String name)
	{
		if (name == "byteLength")
		{
			b.byteLength = ParseIntAbort(p->Token());
		}
		else if (name == "uri")
		{
			auto uri = p->Token();
			b.uri = uri.ToView(1, uri.Length() - 1);
		}
	});
	return b;
}

GLTFBufferView ParseGLTFBufferView(parse::Parser *p)
{
	auto v = GLTFBufferView{};
	JSONParseObject(p, [&v](parse::Parser *p, String name)
	{
		if (name == "buffer")
		{
			v.buffer = ParseIntAbort(p->Token());
		}
		else if (name == "byteOffset")
		{
			v.byteOffset = ParseIntAbort(p->Token());
		}
		else if (name == "byteLength")
		{
			v.byteLength = ParseIntAbort(p->Token());
		}
		else if (name == "byteStride")
		{
			v.byteStride = ParseIntAbort(p->Token());
		}
		else if (name == "target")
		{
			v.target = (GLTFBufferViewTarget)ParseIntAbort(p->Token());
		}
	});
	return v;
}

GLTFMaterial ParseGLTFMaterial(parse::Parser *p)
{
	auto m = GLTFMaterial{};
	JSONParseObject(p, [&m](parse::Parser *p, String name)
	{
		if (name == "pbrMetallicRoughness")
		{
			JSONParseObject(p, [&m](parse::Parser *p, String name)
			{
				if (name == "baseColorFactor")
				{
					auto i = 0;
					JSONParseList(p, [&m, &i](parse::Parser *p)
					{
						m.pbrMetallicRoughness.baseColorFactor[i] = ParseFloatAbort(p->Token());
						i += 1;
					});
				}
				else if (name == "metallicFactor")
				{
					m.pbrMetallicRoughness.metallicFactor = ParseFloatAbort(p->Token());
				}
			});
		}
		else if (name == "name")
		{
			m.name = name;
		}
	});
	return m;
}

GLTFAccessor ParseGLTFAccessor(parse::Parser *p)
{
	auto a = GLTFAccessor{};
	JSONParseObject(p, [&a](parse::Parser *p, String name)
	{
		if (name == "bufferView")
		{
			a.bufferView = ParseIntAbort(p->Token());
		}
		else if (name == "byteOffset")
		{
			a.byteOffset = ParseIntAbort(p->Token());
		}
		else if (name == "componentType")
		{
			a.componentType = (GLTFAccessorComponentType)ParseIntAbort(p->Token());
		}
		else if (name == "count")
		{
			a.count = ParseIntAbort(p->Token());
		}
		else if (name == "type")
		{
			auto t = p->Token();
			if (t == "\"SCALAR\"")
			{
				a.type = GLTFScalarType;
			}
			else if (t == "\"VEC2\"")
			{
				a.type = GLTFVec2Type;
			}
			else if (t == "\"VEC3\"")
			{
				a.type = GLTFVec3Type;
			}
			else if (t == "\"VEC4\"")
			{
				a.type = GLTFVec4Type;
			}
			else if (t == "\"MAT2\"")
			{
				a.type = GLTFMat2Type;
			}
			else if (t == "\"MAT3\"")
			{
				a.type = GLTFMat3Type;
			}
			else if (t == "\"MAT4\"")
			{
				a.type = GLTFMat4Type;
			}
		}
	});
	return a;
}

array::Array<GLTFAttribute> ParseGLTFAttributes(parse::Parser *p)
{
	auto as = array::Array<GLTFAttribute>{};
	JSONParseObject(p, [&as](parse::Parser *p, String name)
	{
		if (name == "NORMAL")
		{
			auto a = GLTFAttribute
			{
				.type = GLTFNormalType,
				.index = ParseIntAbort(p->Token()),
			};
			as.Append(a);
		}
		else if (name == "POSITION")
		{
			auto a = GLTFAttribute
			{
				.type = GLTFPositionType,
				.index = ParseIntAbort(p->Token()),
			};
			as.Append(a);
		}
	});
	return as;
}

GLTFPrimitive ParseGLTFPrimitive(parse::Parser *p)
{
	auto pr = GLTFPrimitive{};
	JSONParseObject(p, [&pr](parse::Parser *p, String name)
	{
		if (name == "attributes")
		{
			pr.attributes = ParseGLTFAttributes(p);
		}
		else if (name == "indices")
		{
			pr.indices = ParseIntAbort(p->Token());
		}
		else if (name == "mode")
		{
			pr.mode = (GLTFPrimitiveMode)ParseIntAbort(p->Token());
		}
		else if (name == "material")
		{
			pr.material = ParseIntAbort(p->Token());
		}
	});
	return pr;
}

GLTFMesh ParseGLTFMesh(parse::Parser *p)
{
	auto m = GLTFMesh{};
	JSONParseObject(p, [&m](parse::Parser *p, String name)
	{
		if (name == "primitives")
		{
			JSONParseList(p, [&m](parse::Parser *p)
			{
				m.primitives.Append(ParseGLTFPrimitive(p));
			});
		}
		else if (name == "name")
		{
			m.name = p->Token();
			m.name = m.name.ToView(1, m.name.Length() - 1); // Strip quotes.
		}
	});
	return m;
}

GLTF ParseGLTFFile(String path, bool *err)
{
	auto p = parse::File(path, "[],{}", err);
	if (*err)
	{
		LogError("GLTF", "Failed creating parser for file %k.", path);
		return GLTF{};
	}
	auto gltf = GLTF{};
	JSONParseObject(&p, [&gltf](parse::Parser *p, String name)
	{
		if (name == "meshes")
		{
			JSONParseList(p, [&gltf](parse::Parser *p)
			{
				gltf.meshes.Append(ParseGLTFMesh(p));
			});
		}
		else if (name == "accessors")
		{
			JSONParseList(p, [&gltf](parse::Parser *p)
			{
				gltf.accessors.Append(ParseGLTFAccessor(p));
			});
		}
		else if (name == "materials")
		{
			JSONParseList(p, [&gltf](parse::Parser *p)
			{
				gltf.materials.Append(ParseGLTFMaterial(p));
			});
		}
		else if (name == "bufferViews")
		{
			JSONParseList(p, [&gltf](parse::Parser *p)
			{
				gltf.bufferViews.Append(ParseGLTFBufferView(p));
			});
		}
		else if (name == "buffers")
		{
			JSONParseList(p, [&gltf](parse::Parser *p)
			{
				gltf.buffers.Append(ParseGLTFBuffer(p));
			});
		}
	});
	return gltf;
}

s64 GLTFComponentTypeToSize(GLTFAccessorComponentType t)
{
	switch (t)
	{
	case GLTFByteType:
	{
		return 1;
	} break;
	case GLTFUnsignedByteType:
	{
		return 1;
	} break;
	case GLTFShortType:
	{
		return 2;
	} break;
	case GLTFUnsignedShortType:
	{
		return 2;
	} break;
	case GLTFUnsignedIntType:
	{
		return 4;
	} break;
	case GLTFFloatType:
	{
		return 4;
	} break;
	}
}

#endif
