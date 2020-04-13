#pragma once

struct ShaderModule
{
	GPUShaderStage stage;
	GPUShaderModule module;
};

struct Shader
{
	Array<ShaderModule> modules;
};

void LoadShaders();
Shader *GetShader(const String &name);
