#pragma once

#if 0

struct Shader
{
	String name;
	Array<GfxShaderModule> modules;
	Array<GfxShaderStage> stages;
};

void LoadShaders();
Shader *GetShader(const String &name);

#endif
