#ifdef VulkanBuild

#include "Shader.h"
#include "Basic/Parser.h"
#include "Basic/File.h"
#include "Basic/Filepath.h"
#include "Basic/Log.h"
#include "Basic/Process.h"

const auto ShaderSourceDirectory = String{"Code/Shader"};

VulkanSPIRV CompileGPUShaderToSPIRV(String filename, bool *err)
{
	CreateDirectoryIfItDoesNotExist("Build/GLSL");
	CreateDirectoryIfItDoesNotExist("Build/GLSL/Code");
	CreateDirectoryIfItDoesNotExist("Build/GLSL/Binary");
	auto shaderPath = JoinFilepaths(ShaderSourceDirectory, filename);
	if (!FileExists(shaderPath))
	{
		LogError("Vulkan", "File %k does not exist.", shaderPath);
		*err = true;
		return {};
	}
	// Write out a new version of the shader, inserting the text for the include files.
	auto procPath = FormatString("Build/GLSL/Code/%k", filename);
	auto procFile = OpenFile(procPath, OpenFileWriteOnly | OpenFileCreate, err);
	if (*err)
	{
		LogError("Vulkan", "Failed to open processed shader output file %k.", procPath);
		return {};
	}
	Defer(procFile.Close());
	auto stageFlags = Array<VkShaderStageFlagBits>{};
	auto stageDefines = Array<String>{};
	auto stageExts = Array<String>{};
	auto fileParser = NewParser(shaderPath, "", err);
	if (*err)
	{
		LogError("Vulkan", "Failed to create parser for shader %k.", shaderPath);
		return {};
	}
	auto depth = 0;
	for (auto line = fileParser.Line(); line != ""; line = fileParser.Line())
	{
		auto lineParser = NewParserFromString(line, "\"");
		auto t = lineParser.Token();
		if (t == "{")
		{
			depth += 1;
		}
		else if (t == "}")
		{
			depth -= 1;
			if (depth == 0)
			{
				procFile.WriteString("#endif\n");
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
			procFile.WriteString("#ifdef ");
			procFile.WriteString(s);
			procFile.WriteString("\n\n");
			t = fileParser.Token();
			if (t != "{")
			{
				LogError("Vulkan", "Failed to parse %k, expected '{' on line %ld after Stage.", shaderPath, fileParser.line);
				*err = true;
				return {};
			}
			else
			{
				depth += 1;
			}
			fileParser.Eat('\n');
		}
		else if (t == "#include")
		{
			if (lineParser.Token() != "\"")
			{
				LogError("Vulkan", "Expected '\"' at %k:%ld:%ld.", shaderPath, fileParser.line, fileParser.column);
				*err = true;
				return {};
			}
			auto includePath = JoinFilepaths("Code/Shader", lineParser.Token());
			if (lineParser.Token() != "\"")
			{
				LogError("Vulkan", "Expected '\"' at %k:%ld:%ld.", shaderPath, fileParser.line, fileParser.column);
				*err = true;
				return {};
			}
			auto includeFile = ReadEntireFile(includePath, err);
			if (*err)
			{
				LogError("Vulkan", "Failed to read include file %k at %k:%ld.", includePath, shaderPath, fileParser.line);
				*err = true;
				return {};
			}
			procFile.Write(includeFile);
			procFile.WriteString("\n");
		}
		else
		{
			procFile.WriteString(line);
		}
	}
	auto spirv = VulkanSPIRV
	{
		.stages = stageFlags,
	};
	auto name = SetFilepathExtension(filename, "");
	for (auto i = 0; i < stageFlags.count; i += 1) 
	{
		auto spirvPath = FormatString("Build/GLSL/Binary/%k%k.spirv", name, stageExts[i]);
		auto cmd = FormatString("glslangValidator -D%k -S %k -V %k -o %k", stageDefines[i], stageExts[i].View(1, stageExts[i].Length()), procPath, spirvPath);
		if (RunProcess(cmd) != 0)
		{
			LogError("Vulkan", "Shader compilation command failed: %k.", cmd);
			*err = true;
			return {};
		}
		auto bc = ReadEntireFile(spirvPath, err);
		if (*err)
		{
			LogError("Vulkan", "Failed to read SPIRV file %k compiled from shader %k.", spirvPath, filename);
			return {};
		}
		spirv.stageByteCode.Append(bc);
	}
	return spirv;
}

#endif
