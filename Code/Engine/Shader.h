#pragma once

// @TODO: Generate this?
union ShaderMaterialParameters {
	union {
		struct {
			u32 albedoMap;
			u32 normalMap;
			u32 roughnessMap;
			u32 metallicMap;
			u32 ambientOcclusionMap;
		} pbr;
		struct {
			V4 color;
			bool colorModified;
		} rustedIron;
	};
};

// @TODO: Generate this?
union ShaderEntityParameters {
	struct {
		M4 modelToWorldSpace;
	} rustedIron;
};

struct Shader {
	ShaderID id;
	String name;
	GPU_Shader gpu;
	Array<GPUDescriptorSet> descriptorSets;
};

Shader *GetShader(ShaderID id);
