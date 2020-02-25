#include "Basic/Basic.h"

s32 main(s32 argc, char *argv[])
{
	if (argc <= 1)
	{
		return 0;
	}

	String shaderBuildDirectory = "Build/Shader";

	for (auto i = 1; i < argc; i++)
	{
		String shaderFilepath = argv[i];
		if (!FileExists(shaderFilepath))
		{
			LogPrint(LogType::ERROR, "shader file %s does not exist\n", &shaderFilepath[0]);
			continue;
		}
		auto [fileContents, error] = ReadEntireFile(shaderFilepath);
		if (error)
		{
			continue;
		}

		ParserStream parser = CreateParserStream(fileContents);
		struct StageInfo
		{
			String stageDefine;
			String stageID;
		};
		Array<StageInfo> stageInfos;
		for (Token token = GetToken; Length(token) > 0; token = GetToken())
		{
			if (token == "VERTEX_SHADER")
			{
				Append(&stageInfos,
				{
					.stageDefine = "VERTEX_SHADER",
					.stageID = "vert",
				});
			}
			if (token == "FRAGMENT_SHADER")
			{
				Append(&stageInfos,
				{
					.stageDefine = "FRAGMENT_SHADER",
					.stageID = "frag",
			    });
			}
		}

		auto fileExtension = GetFilepathExtension(shaderFilepath);
		auto shaderName = GetFilepathFilename(shaderFilepath);
		SetFilepathExtension(&shaderName, "");

		String command;
		if (fileExtension == ".glsl")
		{
			command = _FormatString("$VULKAN_SDK_PATH/bin/glslangValidator -V %s -D%s -S %s -o Build/Shader/GLSL/Binary/%s.%s.spirv", &shaderFilepath[0], &stageDefine[0], &stageID[0], &shaderName[0], &stageID[0]);
		}
		else
		{
			InvalidCodePath();
		}
		if (system(command) != 0)
		{
			LogPrint(LogType::ERROR, "shader compilation command failed: %s", command);
			continue;
		}
	}
}
