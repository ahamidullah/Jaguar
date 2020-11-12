#if DevelopmentBuild

#include "GLTF.h"
#include "Basic/JSON.h"

f32 ParseFloatAbort(string::String s)
{
	auto err = false;
	auto f = ParseFloat(s, &err);
	if (err)
	{
		Abort("String", "Failed to parse float %k.", s);
	}
	return f;
}

s64 ParseIntAbort(string::String s)
{
	auto err = false;
	auto n = string::ParseInt(s, &err);
	if (err)
	{
		Abort("String", "Failed parse integer for string %k.", s);
	}
	return n;
}

GLTFBuffer ParseGLTFBuffer(parser::Parser *p)
{
	auto b = GLTFBuffer{};
	JSONParseObject(p, [&b](parser::Parser *p, string::String name)
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

GLTFBufferView ParseGLTFBufferView(parser::Parser *p)
{
	auto v = GLTFBufferView{};
	JSONParseObject(p, [&v](parser::Parser *p, string::String name)
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

GLTFMaterial ParseGLTFMaterial(parser::Parser *p)
{
	auto m = GLTFMaterial{};
	JSONParseObject(p, [&m](parser::Parser *p, string::String name)
	{
		if (name == "pbrMetallicRoughness")
		{
			JSONParseObject(p, [&m](parser::Parser *p, string::String name)
			{
				if (name == "baseColorFactor")
				{
					auto i = 0;
					JSONParseList(p, [&m, &i](parser::Parser *p)
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

GLTFAccessor ParseGLTFAccessor(parser::Parser *p)
{
	auto a = GLTFAccessor{};
	JSONParseObject(p, [&a](parser::Parser *p, string::String name)
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

array::Array<GLTFAttribute> ParseGLTFAttributes(parser::Parser *p)
{
	auto as = array::Array<GLTFAttribute>{};
	JSONParseObject(p, [&as](parser::Parser *p, string::String name)
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

GLTFPrimitive ParseGLTFPrimitive(parser::Parser *p)
{
	auto pr = GLTFPrimitive{};
	JSONParseObject(p, [&pr](parser::Parser *p, string::String name)
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

GLTFMesh ParseGLTFMesh(parser::Parser *p)
{
	auto m = GLTFMesh{};
	JSONParseObject(p, [&m](parser::Parser *p, string::String name)
	{
		if (name == "primitives")
		{
			JSONParseList(p, [&m](parser::Parser *p)
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

GLTF ParseGLTFFile(string::String path, bool *err)
{
	auto p = parser::NewFromFile(path, "[],{}", err);
	if (*err)
	{
		LogError("GLTF", "Failed creating parser for file %k.", path);
		return GLTF{};
	}
	auto gltf = GLTF{};
	JSONParseObject(&p, [&gltf](parser::Parser *p, string::String name)
	{
		if (name == "meshes")
		{
			JSONParseList(p, [&gltf](parser::Parser *p)
			{
				gltf.meshes.Append(ParseGLTFMesh(p));
			});
		}
		else if (name == "accessors")
		{
			JSONParseList(p, [&gltf](parser::Parser *p)
			{
				gltf.accessors.Append(ParseGLTFAccessor(p));
			});
		}
		else if (name == "materials")
		{
			JSONParseList(p, [&gltf](parser::Parser *p)
			{
				gltf.materials.Append(ParseGLTFMaterial(p));
			});
		}
		else if (name == "bufferViews")
		{
			JSONParseList(p, [&gltf](parser::Parser *p)
			{
				gltf.bufferViews.Append(ParseGLTFBufferView(p));
			});
		}
		else if (name == "buffers")
		{
			JSONParseList(p, [&gltf](parser::Parser *p)
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
