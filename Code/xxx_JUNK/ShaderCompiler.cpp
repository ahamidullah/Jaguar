#include "Basic/Basic.h"

// @TODO: Output pipeline creation code.
// @TODO: Output descriptor set creation code.

struct InputInfo
{
	String location;
	String type;
	String name;
};

struct UniformInfo
{
	String layout;
	String set;
	String binding;
	String name;
};

void ParseInputOutput(ParserStream *parser, Array<InputInfo> *inputInfos)
{
	GetExpectedToken(parser, "=");
	auto location = GetToken(parser);
	GetExpectedToken(parser, ")");
	auto token = GetToken(parser);
	if (token != "in")
	{
		return;
	}
	auto type = GetToken(parser);
	auto name = GetToken(parser);
	Append(inputInfos, 
	{
		.location = location,
		.type = type,
		.name = name,
	});
}

void OutputPipelineCreationCode(FileHandle file, String shaderName, Array<InputInfo> *inputInfos)
{
	WriteStringToFile(file, "GPUPipeline CreateShaderGraphicsPipeline(const String &shaderName, GPURenderPass renderPass, const ShaderDescriptorSets &descriptorSets, const Array<ShaderModule> &modules)\n");
	WriteStringToFile(file, "{\n");
	WriteStringToFile(file, FormatString("	if (shaderName == \"%s\")\n", &shaderName[0]));
	WriteStringToFile(file, "	{\n");
	WriteStringToFile(file,
		"		GPUFramebufferAttachmentColorBlendDescription colorBlendDescription =\n"
		"		{\n"
		"			.enable_blend = true,\n"
		"			.source_color_blend_factor = GPU_BLEND_FACTOR_SRC_ALPHA,\n"
		"			.destination_color_blend_factor = GPU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,\n"
		"			//.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,\n"
		"			//.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,\n"
		"			.color_blend_operation = GPU_BLEND_OP_ADD,\n"
		"			.source_alpha_blend_factor = GPU_BLEND_FACTOR_ONE,\n"
		"			.destination_alpha_blend_factor = GPU_BLEND_FACTOR_ZERO,\n"
		"			.alpha_blend_operation = GPU_BLEND_OP_ADD,\n"
		"			.color_write_mask = GPU_COLOR_COMPONENT_RED | GPU_COLOR_COMPONENT_GREEN | GPU_COLOR_COMPONENT_BLUE | GPU_COLOR_COMPONENT_ALPHA,\n"
		"		};\n");
	WriteStringToFile(file,
		"		GPUPipelineVertexInputBindingDescription vertexInputBindingDescriptions[] =\n"
		"		{\n"
		"			{\n"
		"				.binding = GPU_VERTEX_BUFFER_BIND_ID,\n"
		"				.stride = sizeof(Vertex1P1N),\n"
		"				.input_rate = GPU_VERTEX_INPUT_RATE_VERTEX,\n"
		"			},\n"
		"		};\n");
	WriteStringToFile(file,
		"		GPUPipelineVertexInputAttributeDescription vertexInputAttributeDescriptions[] =\n"
		"		{\n");
	for (auto info : *inputInfos)
	{
		s64 locationNumber;
		Assert(ParseInteger(info.location, &locationNumber));
		auto structMemberName = info.name;
		Trim(&structMemberName, 5, Length(structMemberName));
		LowercaseString(&structMemberName);
		WriteStringToFile(file,
			FormatString(
				"			{\n"
				"				.format = GPU_FORMAT_R32G32B32_SFLOAT,\n"
				"				.binding = GPU_VERTEX_BUFFER_BIND_ID,\n"
				"				.location = %d,\n"
				"				.offset = offsetof(Vertex1P1N, %s),\n"
				"			},\n",
				locationNumber, &structMemberName[0]));
	}
	WriteStringToFile(file, "		}\n");
	WriteStringToFile(file,
		"		GPUDynamicPipelineState dynamicStates[] =\n"
		"		{\n"
		"			GPU_DYNAMIC_PIPELINE_STATE_VIEWPORT,\n"
		"			GPU_DYNAMIC_PIPELINE_STATE_SCISSOR,\n"
		"		};\n");
	WriteStringToFile(file,
		"		GPUPipelineDescription pipelineDescription =\n"
		"		{\n"
		"			.descriptor_set_layout_count = 1,\n"
		"			.descriptor_set_layouts = descriptorSet.layout,\n"
		"			.push_constant_count = 0,\n"
		"			.push_constant_descriptions = NULL,\n"
		"			.topology = GPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,\n"
		"			.viewport_width = (f32)windowWidth,\n"
		"			.viewport_height = (f32)windowHeight,\n"
		"			.scissor_width = windowWidth,\n"
		"			.scissor_height = windowHeight,\n"
		"			.depth_compare_operation = GPU_COMPARE_OP_LESS,\n"
		"			.framebuffer_attachment_color_blend_count = 1,\n"
		"			.framebuffer_attachment_color_blend_descriptions = &colorBlendDescription,\n"
		"			.vertex_input_attribute_count = ArrayCount(vertexInputAttributeDescriptions),\n"
		"			.vertex_input_attribute_descriptions = vertexInputAttributeDescriptions,\n"
		"			.vertex_input_binding_count = ArrayCount(vertexInputBindingDescriptions),\n"
		"			.vertex_input_binding_descriptions = vertexInputBindingDescriptions,\n"
		"			.dynamic_state_count = ArrayCount(dynamicStates),\n"
		"			.dynamic_states = dynamicStates,\n"
		"			.shader = modules,\n"
		"			.render_pass = renderPass,\n"
		"			.enable_depth_bias = false,\n"
		"		};\n");
	WriteStringToFile(file, "		return GPUCreatePipeline(&pipelineDescription);\n");
	WriteStringToFile(file, "	}\n\n");
	WriteStringToFile(file, "}\n\n");
}

void ProcessShader(const String &filepath)
{
	xxxxx
}

s32 ApplicationEntry(s32 argc, char *argv[])
{
	String shaderBuildDirectory = "Build/Shader";

	if (argc > 1)
	{
		ProcessShader(argv[1]);
	}
	else
	{
		String shaderDirectory = "Code/Shader/GLSL";
		DirectoryIteration iteration;
		while (IterateDirectory(shaderDirectory, &iteration))
		{
			if (GetFilepathExtension(iteration.filename) != ".glsl")
			{
				continue;
			}
			ProcessShader(JoinFilepaths(shaderDirectory, iteration.filename));
		}
	}

	for (auto i = 1; i < argc; i++)
	{
		String shaderFilepath = argv[i];
		if (!FileExists(shaderFilepath))
		{
			LogPrint(LogType::ERROR, "shader file %s does not exist\n", &shaderFilepath[0]);
			continue;
		}
		ParserStream parser;
		if (!CreateParserStream(&parser, shaderFilepath))
		{
			LogPrint(LogType::ERROR, "failed to create parser: %s\n", &shaderFilepath[0]);
			continue;
		}
		struct StageInfo
		{
			String stageDefine;
			String stageID;
		};
		Array<StageInfo> stageInfos;
		Array<InputInfo> inputInfos;
		String currentStage;
		for (String token = GetToken(&parser); Length(token) > 0; token = GetToken(&parser))
		{
			if (token == "VERTEX_SHADER")
			{
				Append(&stageInfos,
				{
					.stageDefine = "VERTEX_SHADER",
					.stageID = "vert",
				});
				currentStage = "VERTEX";
			}
			if (token == "FRAGMENT_SHADER")
			{
				Append(&stageInfos,
				{
					.stageDefine = "FRAGMENT_SHADER",
					.stageID = "frag",
			    });
			    currentStage = "FRAGMENT";
			}
			if (token == "layout")
			{
				GetExpectedToken(&parser, "(");
				if (GetToken(&parser) == "location" && currentStage == "VERTEX") // input or output
				{
					ParseInputOutput(&parser, &inputInfos);
				}
				else // uniform
				{
					//ParseUniform(&parser, &uniformInfos);
				}
			}
		}

		auto fileExtension = GetFilepathExtension(shaderFilepath);
		auto shaderName = GetFilepathFilename(shaderFilepath);
		SetFilepathExtension(&shaderName, "");

		auto [file, error] = OpenFile("Code/Engine/Shader.generated.cpp", OPEN_FILE_WRITE_ONLY | OPEN_FILE_CREATE);
		Assert(!error);
		OutputPipelineCreationCode(file, shaderName, &inputInfos);

		for (const auto &info : stageInfos)
		{
			String command;
			if (fileExtension == ".glsl")
			{
				command = FormatString("$VULKAN_SDK_PATH/bin/glslangValidator -V %s -D%s -S %s -o Build/Shader/GLSL/Binary/%s.%s.spirv", &shaderFilepath[0], &info.stageDefine[0], &info.stageID[0], &shaderName[0], &info.stageID[0]);
			}
			else
			{
				InvalidCodePath();
			}
			if (RunProcess(command) != 0)
			{
				LogPrint(LogType::ERROR, "shader compilation command failed: %s", command);
				continue;
			}
		}
	}
	return 1;
}
