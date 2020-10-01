#ifdef VulkanBuild

#include "Shader.h"
#include "Basic/Parser.h"
#include "Basic/File.h"
#include "Basic/Filepath.h"

Array<String> GPUShaderFilepaths()
{
	auto a = Array<String>{};
	auto shaderDir = String{"Code/Shader"};
	auto itr = DirectoryIteration{};
	while (itr.Iterate(shaderDir))
	{
		auto fp = JoinFilepaths(shaderDir, itr.filename);
		auto ext = FilepathExtension(fp);
		if (ext != ".glsl")
		{
			continue;
		}
		a.Append(fp);
	}
	return a;
}

VulkanSPIRVInfo GenerateVulkanSPIRV(String filepath, bool *err)
{
	if (!FileExists(filepath))
	{
		LogError("Vulkan", "File %k does not exist.", filepath);
		*err = true;
		return {};
	}
	// Write out a new version of the shader, inserting the text for the include files.
	auto ppFilepath = FormatString("Build/Linux/Shader/Code/%k", FilepathFilename(filepath));
	auto ppFile = OpenFile(ppFilepath, OpenFileWriteOnly | OpenFileCreate, err);
	if (*err)
	{
		LogError("Vulkan", "Failed to open preprocessed shader output file %k.", ppFilepath);
		return {};
	}
	Defer(ppFile.Close());
	auto stageFlags = Array<VkShaderStageFlagBits>{};
	auto stageDefines = Array<String>{};
	auto stageExts = Array<String>{};
	auto fileParser = NewParser(filepath, "", err);
	if (*err)
	{
		LogError("Vulkan", "Failed to create parser for shader %k.", filepath);
		return {};
	}
	auto braceDepth = 0;
	for (auto line = fileParser.Line(); line != ""; line = fileParser.Line())
	{
		auto lineParser = NewParserFromString(line, "\"");
		auto t = lineParser.Token();
		if (t == "{")
		{
			braceDepth += 1;
		}
		else if (t == "}")
		{
			braceDepth -= 1;
			if (braceDepth == 0)
			{
				ppFile.WriteString("#endif\n");
				continue;
			}
		}
		if (t == "Stage:")
		{
			auto s = lineParser.Token();
			stageDefines.Append(s);
			if (s == "Vertex")
			{
				stageFlags.Append(VK_SHADER_STAGE_VERTEX_BIT);
				stageExts.Append(".vert");
			}
			else if (s == "Fragment")
			{
				stageFlags.Append(VK_SHADER_STAGE_FRAGMENT_BIT);
				stageExts.Append(".frag");
			}
			else if (s == "Compute")
			{
				stageFlags.Append(VK_SHADER_STAGE_COMPUTE_BIT);
				stageExts.Append(".comp");
			}
			else
			{
				LogError("Vulkan", "Unknown Stage: %k.", s);
				return {};
			}
			ppFile.WriteString("#ifdef ");
			ppFile.WriteString(s);
			ppFile.WriteString("\n\n");
			t = fileParser.Token();
			if (t != "{")
			{
				LogError("Vulkan", "Failed to parse %k, expected '{' on line %ld after Stage.", filepath, fileParser.line);
				*err = true;
				return {};
			}
			else
			{
				braceDepth += 1;
			}
			fileParser.Eat('\n');
		}
		else if (t == "#include")
		{
			if (lineParser.Token() != "\"")
			{
				LogError("Vulkan", "Expected '\"' at %k:%ld:%ld.", filepath, lineParser.line, lineParser.column);
				*err = true;
				return {};
			}
			auto includeFilepath = JoinFilepaths("Code/Shader", lineParser.Token());
			if (lineParser.Token() != "\"")
			{
				LogError("Vulkan", "Expected '\"' at %k:%ld:%ld.", filepath, lineParser.line, lineParser.column);
				*err = true;
				return {};
			}
			auto includeFileContents = ReadEntireFile(includeFilepath, err);
			if (*err)
			{
				LogError("Vulkan", "Failed to read include file %k at %k:%ld.", includeFilepath, filepath, lineParser.line);
				*err = true;
				return {};
			}
			ppFile.WriteString(includeFileContents);
			ppFile.WriteString("\n");
		}
		else
		{
			ppFile.WriteString(line);
		}
	}
	auto name = SetFilepathExtension(FilepathFilename(filepath), "");
	auto si = VulkanSPIRVInfo
	{
		.name = name,
		.stages = stageFlags,
	};
	for (auto i = 0; i < stageFlags.count; i += 1) 
	{
		auto spirvFilepath = FormatString("Build/Linux/Shader/Binary/%k%k.spirv", name, stageExts[i]);
		auto cmd = FormatString("glslangValidator -D%k -S %k -V %k -o %k", stageDefines[i], stageExts[i].View(1, stageExts[i].Length()), ppFilepath, spirvFilepath);
		if (RunProcess(cmd) != 0)
		{
			LogError("Vulkan", "Shader compilation command failed: %k.", cmd);
			*err = true;
			return {};
		}
		si.filepaths.Append(spirvFilepath);
	}
	return si;
}

#endif
