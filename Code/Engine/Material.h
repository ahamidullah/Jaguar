#pragma once

struct Material {
	MaterialID id;
	String name;
	ShaderID shaderID;
	ShaderMaterialParameters shaderParameters;
}
