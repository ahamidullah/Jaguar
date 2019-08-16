#version 420

layout(push_constant) uniform PER_OBJECT {
	int diffuse_map_texture_index;
	int normal_map_texture_index;
} push_constants;

layout(binding = 2, set = 0) uniform sampler diffuse_sampler;
layout(binding = 3, set = 0) uniform sampler2D shadow_map_texture;
layout(binding = 4, set = 2) uniform texture2D textures[100];

layout(location = 0) in vec3 world_space_position;
layout(location = 1) in vec4 light_space_position;
layout(location = 2) in vec2 uv;
//layout(location = 3) in vec3 tangent_fragment_position;
layout(location = 3) in vec3 light_direction;
layout(location = 4) in vec3 fragment_normal;
layout(location = 5) in vec3 lll;

layout(location = 0) out vec4 color;

vec3 ambient_intensity = vec3(0.10f);
vec3 diffuse_intensity = vec3(0.6f);
//vec3 directional_light_direction = vec3(0.0, 0.0, 1.0);//-0.832050, 0.000000, -0.554700);//-2.2f, 0.0f, -1.3f);

float calculate_shadows(vec4 light_space_position) {
	float bias = 0.000;//max(0.005 * (1.0 - dot(normal, directional_light_direction)), 0.005);
	vec3 p = light_space_position.xyz / light_space_position.w;
	if (p.z > 1.0) {
		return 1.0;
	} else if (p.z - bias > texture(shadow_map_texture, p.xy).r) {
		return 0.0;
	}
	return 1.0;
}

vec3 ambient(vec3 intensity) {
	//if (has_texture) {
		return intensity * vec3(1.0, 0.0, 0.0);//vec3(texture(sampler2D(textures[push_constants.diffuse_map_texture_index], diffuse_sampler), uv));
	//}
	//return intensity * fs_in.color;
}

vec3 diffuse(vec3 intensity, vec3 receive_direction, vec3 normal) {
	float impact = max(dot(normal, receive_direction), 0.0f);
	//if (has_texture) {
		return intensity * impact * vec3(1.0, 0.0, 0.0);//vec3(texture(sampler2D(textures[push_constants.diffuse_map_texture_index], diffuse_sampler), uv));
	//}
	//return intensity * impact * fs_in.color;
}

void main() {
	vec3 normal = texture(sampler2D(textures[push_constants.normal_map_texture_index], diffuse_sampler), uv).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	vec3 directional_lighting_result = vec3(0);
	float shadowed = calculate_shadows(light_space_position);

	// Directional Lighting
	{
		vec3 receive_direction = normalize(-light_direction);
		directional_lighting_result = ambient(ambient_intensity)
		                            + (shadowed * diffuse(diffuse_intensity, receive_direction, normal));
	}

	color = vec4(directional_lighting_result, 1.0);
}
