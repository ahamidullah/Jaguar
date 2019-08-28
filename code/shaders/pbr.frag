#version 420
#extension GL_EXT_nonuniform_qualifier : enable

struct Material {
	uint albedo_map_texture_index;
	uint normal_map_texture_index;
	uint metallic_map_texture_index;
	uint roughness_map_texture_index;
	uint ao_map_texture_index;
};

// "Note that it is valid for multiple descriptor arrays in a shader to use the same set and binding number, as long as they are all compatible with the descriptor type in the pipeline layout. This means a single array binding in the descriptor set can serve multiple texture dimensionalities, or an array of buffer descriptors can be used with multiple different block layouts."
layout(std140, binding = 5, set = 4) uniform Materials {
	Material materials[100];
};

layout(push_constant) uniform PER_OBJECT {
	int real_normal_map;
	int albedo_map_texture_index;
	int normal_map_texture_index;
	int metallic_map_texture_index;
	int roughness_map_texture_index;
	int ao_map_texture_index;
} push_constants;

layout(binding = 2, set = 0) uniform sampler diffuse_sampler;
layout(binding = 3, set = 0) uniform sampler2D shadow_map_texture;
layout(binding = 4, set = 2) uniform texture2D textures[];

layout(location = 0) in vec3 fragment_tangent_space_position;
layout(location = 1) in vec4 fragment_light_space_position;
layout(location = 2) in vec2 fragment_uv;
layout(location = 3) in vec3 light_tangent_space_direction;
layout(location = 4) in vec3 fragment_normal;
layout(location = 5) flat in uint fragment_material_id;
layout(location = 6) in vec3 camera_tangent_space_position;

layout(location = 0) out vec4 fragment_color;

//in vec3 WorldPos;
//in vec3 Normal;

// material parameters
//uniform vec3  albedo;
//uniform float metallic;
//uniform float roughness;
//uniform float ao;
//const vec3  albedo = vec3(1.0, 0.0, 0.0);
//const float metallic = 1.0;
//const float roughness = 0.4;
//const float ao = 0.1;
//uniform sampler2D albedoMap;
//uniform sampler2D normalMap;
//uniform sampler2D metallicMap;
//uniform sampler2D roughnessMap;
//uniform sampler2D aoMap;

// lights
//uniform vec3 lightPositions[4];
//uniform vec3 lightColors[4];

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    //vec3 albedo     = pow(texture(sampler2D(textures[push_constants.albedo_map_texture_index], diffuse_sampler), fragment_uv).rgb, vec3(2.2));
    //vec3 normal     = texture(sampler2D(textures[push_constants.normal_map_texture_index], diffuse_sampler), fragment_uv).rgb;//getNormalFromNormalMap();
    //float metallic  = texture(sampler2D(textures[push_constants.metallic_map_texture_index], diffuse_sampler), fragment_uv).r;
    //float roughness = texture(sampler2D(textures[push_constants.roughness_map_texture_index], diffuse_sampler), fragment_uv).r;
    //float ao        = texture(sampler2D(textures[push_constants.ao_map_texture_index], diffuse_sampler), fragment_uv).r;

    vec3 albedo     = pow(texture(sampler2D(textures[materials[fragment_material_id].albedo_map_texture_index], diffuse_sampler), fragment_uv).rgb, vec3(2.2));
    //vec3 normal     = texture(sampler2D(textures[materials[fragment_material_id].normal_map_texture_index], diffuse_sampler), fragment_uv).rgb;//getNormalFromNormalMap();
    float metallic  = texture(sampler2D(textures[materials[fragment_material_id].metallic_map_texture_index], diffuse_sampler), fragment_uv).r;
    float roughness = texture(sampler2D(textures[materials[fragment_material_id].roughness_map_texture_index], diffuse_sampler), fragment_uv).r;
    float ao        = texture(sampler2D(textures[materials[fragment_material_id].ao_map_texture_index], diffuse_sampler), fragment_uv).r;

	//normal = normal * 2.0 - vec3(1.0, 1.0, 2.0);

	vec3 real_normal = texture(sampler2D(textures[materials[fragment_material_id].normal_map_texture_index], diffuse_sampler), fragment_uv).rgb;
	real_normal = normalize(real_normal * 2.0 - 1.0);
	//real_normal = (normalize(normal + real_normal));

	vec3 N = real_normal;//normalize(fragment_normal);
	vec3 V = normalize(camera_tangent_space_position - fragment_tangent_space_position);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	// reflectance equation
	vec3 Lo = vec3(0.0);
	vec3 lightColor = vec3(1.0);
	//for(int i = 0; i < 4; ++i)
	//{
	// calculate per-light radiance
	vec3 L = normalize(-light_tangent_space_direction);//normalize(lll - fragment_world_space_position);
	vec3 H = normalize(V + L);
	float distance    = length(light_tangent_space_direction - fragment_tangent_space_position);
	float attenuation = 1.0;//1.0 / (distance * distance);
	vec3 radiance     = lightColor * attenuation;

	// cook-torrance brdf
	float NDF = DistributionGGX(N, H, roughness);
	float G   = GeometrySmith(N, V, L, roughness);
	vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	vec3 numerator    = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular     = numerator / max(denominator, 0.001);

	// add to outgoing radiance Lo
	float NdotL = max(dot(N, L), 0.0);
	Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	//}

	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + Lo;

	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));

	fragment_color = vec4(color, 1.0);
}
