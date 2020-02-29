#pragma once

enum ShaderID
{
	FLAT_COLOR_SHADER,
	SHADER_COUNT,
};

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
Shader *GetShader(ShaderID id);
