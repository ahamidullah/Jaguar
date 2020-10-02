#if DevelopmentBuild

#include "GLTF.h"
#include "Basic/Parser.h"
//#include "Basic/JSON.h"

void JSONEatFieldValue(Parser *p)
{
	auto t = p->Token();
	if (t == "{")
	{
		auto depth = 1;
		for (auto c = p->PeekChar(); c != -1 && depth > 0; c = p->PeekChar())
		{
			if (c == '}')
			{
				depth -= 1;
			}
			else if (c == '{')
			{
				depth += 1;
			}
			p->Advance();
		}
	}
	else if (t == "[")
	{
		auto depth = 1;
		for (auto c = p->PeekChar(); c != -1 && depth > 0; c = p->PeekChar())
		{
			if (c == ']')
			{
				depth -= 1;
			}
			else if (c == '[')
			{
				depth += 1;
			}
			p->Advance();
		}
	}
	else
	{
		// Eat the value, which can only be a string, a number, 'true', 'false', or 'null'.
	}
}

template <typename F>
void JSONParseObject(Parser *p, F &&proc)
{
	p->Expect('{');
	for (auto t = p->Token(); t != "" && t != "}"; t = p->Token())
	{
		auto name = t.View(1, t.Length() - 2);
		auto start = p->index;
		proc(p, name);
		if (p->index == start)
		{
			// User did not care about the value, so just eat it.
			JSONEatFieldValue(p);
		}
		if (p->PeekChar() == ',')
		{
			p->Advance();
		}
	}
}

template <typename F>
void JSONParseList(Parser *p, F &&proc)
{
	p->Expect('[');
	auto c = p->PeekChar();
	for (; c != -1 && c != ']'; c = p->PeekChar())
	{
		if (c == ',')
		{
			p->Advance();
		}
		auto start = p->index;
		proc(p);
		Assert(p->index != start);
	}
	if (c == ']')
	{
		p->Advance();
	}
}

GLTFBuffer GLTFParseBuffer(Parser *p)
{
	auto b = GLTFBuffer{};
	JSONParseObject(p, [&b](Parser *p, String name)
	{
		if (name == "byteLength")
		{
			b.byteLength = ParseIntegerAbort(p->Token());
		}
		else if (name == "uri")
		{
			b.uri = p->Token();
		}
	});
	return b;
}

GLTFBufferView GLTFParseBufferView(Parser *p)
{
	auto v = GLTFBufferView{};
	JSONParseObject(p, [&v](Parser *p, String name)
	{
		if (name == "buffer")
		{
			v.buffer = ParseIntegerAbort(p->Token());
		}
		else if (name == "byteOffset")
		{
			v.byteOffset = ParseIntegerAbort(p->Token());
		}
		else if (name == "byteLength")
		{
			v.byteLength = ParseIntegerAbort(p->Token());
		}
		else if (name == "byteStride")
		{
			v.byteStride = ParseIntegerAbort(p->Token());
		}
		else if (name == "target")
		{
			v.target = (GLTFBufferViewTarget)ParseIntegerAbort(p->Token());
		}
	});
	return v;
}

GLTFMaterial GLTFParseMaterial(Parser *p)
{
	auto m = GLTFMaterial{};
	JSONParseObject(p, [&m](Parser *p, String name)
	{
		if (name == "pbrMetallicRoughness")
		{
			JSONParseObject(p, [&m](Parser *p, String name)
			{
				if (name == "baseColorFactor")
				{
					auto i = 0;
					JSONParseList(p, [&m, &i](Parser *p)
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

GLTFAccessor GLTFParseAccessor(Parser *p)
{
	auto a = GLTFAccessor{};
	JSONParseObject(p, [&a](Parser *p, String name)
	{
		if (name == "bufferView")
		{
			a.bufferView = ParseIntegerAbort(p->Token());
		}
		else if (name == "byteOffset")
		{
			a.byteOffset = ParseIntegerAbort(p->Token());
		}
		else if (name == "componentType")
		{
			a.componentType = (GLTFAccessorComponentType)ParseIntegerAbort(p->Token());
		}
		else if (name == "count")
		{
			a.count = ParseIntegerAbort(p->Token());
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

Array<GLTFAttribute> GLTFParseAttributes(Parser *p)
{
	auto as = Array<GLTFAttribute>{};
	JSONParseObject(p, [&as](Parser *p, String name)
	{
		if (name == "NORMAL")
		{
			auto a = GLTFAttribute
			{
				.type = GLTFNormalType,
				.index = ParseIntegerAbort(p->Token()),
			};
			as.Append(a);
		}
		else if (name == "POSITION")
		{
			auto a = GLTFAttribute
			{
				.type = GLTFPositionType,
				.index = ParseIntegerAbort(p->Token()),
			};
			as.Append(a);
		}
	});
	return as;
}

GLTFPrimitive GLTFParsePrimitive(Parser *p)
{
	auto pr = GLTFPrimitive{};
	JSONParseObject(p, [&pr](Parser *p, String name)
	{
		if (name == "attributes")
		{
			pr.attributes = GLTFParseAttributes(p);
		}
		else if (name == "indices")
		{
			pr.indices = ParseIntegerAbort(p->Token());
		}
		else if (name == "mode")
		{
			pr.mode = (GLTFPrimitiveMode)ParseIntegerAbort(p->Token());
		}
		else if (name == "material")
		{
			pr.material = ParseIntegerAbort(p->Token());
		}
	});
	return pr;
}

GLTFMesh GLTFParseMesh(Parser *p)
{
	auto m = GLTFMesh{};
	JSONParseObject(p, [&m](Parser *p, String name)
	{
		if (name == "primitives")
		{
			JSONParseList(p, [&m](Parser *p)
			{
				m.primitives.Append(GLTFParsePrimitive(p));
			});
		}
		else if (name == "name")
		{
			m.name = p->Token();
			m.name = m.name.View(1, m.name.Length() - 1); // Strip quotes.
		}
	});
	return m;
}

GLTF GLTFParseFile(String path, bool *err)
{
	auto p = NewParser(path, "[],{}", err);
	if (*err)
	{
		LogError("GLTF", "Failed creating parser for file %k.", path);
		return {};
	}
	auto gltf = GLTF{};
	JSONParseObject(&p, [&gltf](Parser *p, String name)
	{
		if (name == "meshes")
		{
			JSONParseList(p, [&gltf](Parser *p)
			{
				gltf.meshes.Append(GLTFParseMesh(p));
			});
		}
		else if (name == "accessors")
		{
			JSONParseList(p, [&gltf](Parser *p)
			{
				gltf.accessors.Append(GLTFParseAccessor(p));
			});
		}
		else if (name == "materials")
		{
			JSONParseList(p, [&gltf](Parser *p)
			{
				gltf.materials.Append(GLTFParseMaterial(p));
			});
		}
		else if (name == "bufferViews")
		{
			JSONParseList(p, [&gltf](Parser *p)
			{
				gltf.bufferViews.Append(GLTFParseBufferView(p));
			});
		}
		else if (name == "buffers")
		{
			JSONParseList(p, [&gltf](Parser *p)
			{
				gltf.buffers.Append(GLTFParseBuffer(p));
			});
		}
	});
	return gltf;
}

#endif
