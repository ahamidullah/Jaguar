static Shader shaders[SHADER_COUNT];

struct CreateShaderInfo
{
	GPUShaderStage stage;
	String spirv;
};

void CreateShader(const Array<CreateShaderInfo> &infos, Shader *shader)
{
	for (auto i : infos)
	{
		Append(&shader->modules,
		{
			.stage = i.stage,
			.module = GPUCreateShaderModule(i.stage, i.spirv),
		});
	}
}

void LoadShaders()
{
	if (development)
	{
		String shaderDirectory;
		String binaryDirectory;
		String binaryFilenameExtension;
		if (usingVulkanAPI)
		{
			shaderDirectory = "Code/Shader/GLSL";
			binaryDirectory = "Build/Shader/GLSL/Binary";
			binaryFilenameExtension = "spirv";
		}
		else
		{
			InvalidCodePath();
		}

		DirectoryIteration iteration;
		while (IterateDirectory(shaderDirectory, &iteration))
		{
			auto sourceFilepath = JoinFilepaths(shaderDirectory, iteration.filename);
			RunProcess(_FormatString("ShaderCompiler %s", &sourceFilepath[0]));
			String moduleNames[] = {
				"vert",
				"frag",
				"comp",
			};
			Array<CreateShaderInfo> infos;
			for (auto &module : moduleNames)
			{
				auto binaryFilepath = JoinFilepaths(binaryDirectory, iteration.filename);
				SetFilepathExtension(&binaryFilepath, _FormatString(".%s.%s", &module[0], &binaryFilenameExtension[0]));
				if (!FileExists(binaryFilepath))
				{
					continue;
				}
				auto [spirv, error] = ReadEntireFile(binaryFilepath);
				if (error)
				{
					continue;
				}
				GPUShaderStage stage;
				if (module == "vert")
				{
					stage = GPU_VERTEX_SHADER_STAGE;
				}
				else if (module == "frag")
				{
					stage = GPU_FRAGMENT_SHADER_STAGE;
				}
				else if (module == "comp")
				{
					stage = GPU_COMPUTE_SHADER_STAGE;
				}
				else
				{
					InvalidCodePath();
				}
				Append(&infos,
				{
					.stage = stage,
					.spirv = spirv,
				});
			}
			CreateShader(infos, &shaders[0]);
		}
	}
	else
	{
		InvalidCodePath(); // Should just load the binaries from somewhere...
	}
}

Shader *GetShader(ShaderID id)
{
	return &shaders[id];
}
