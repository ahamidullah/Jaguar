#version 420

/*
//layout(location = 0) in vec3 fragColor;
//layout(location = 1) in vec2 fragTexCoord;

//const vec3 directional_light_direction = vec3(-0.707107, -0.707107, -0.707107);//-0.832050, 0.000000, -0.554700);//-2.2f, 0.0f, -1.3f);

in VS_OUT {
	vec4 color;
	vec4 uv;
	vec4 light_space_position;
} fs_in;

out vec4 color;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}
*/

//uniform bool has_texture;
layout(binding = 2) uniform sampler diffuse_sampler;
layout(binding = 3) uniform sampler2D shadow_map_texture;
layout(binding = 4) uniform texture2D textures[100];

layout(push_constant) uniform PER_OBJECT {
	int imgIdx;
} pc;

layout(location = 0) in vec3 world_space_position;
layout(location = 1) in vec4 light_space_position;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec4 color;

vec3 ambient_intensity = vec3(0.15f, 0.15f, 0.15f);
vec3 diffuse_intensity = vec3(0.6f, 0.6f, 0.6f);
vec3 directional_light_direction = vec3(-0.707107, -0.707107, -0.707107);//-0.832050, 0.000000, -0.554700);//-2.2f, 0.0f, -1.3f);

float calculate_shadows(vec4 light_space_position) {
	float bias = 0.000;//max(0.005 * (1.0 - dot(normal, directional_light_direction)), 0.005);

	vec3 p = light_space_position.xyz / light_space_position.w;
	//p.x = 0.5 * p.x + 0.5;
	//p.y = 0.5 * p.y + 0.5;
	if (p.z > 1.0) {
		return 1.0;
	} else if (p.z - bias > texture(shadow_map_texture, p.xy).r) {
		return 0.0;
	}
	return 1.0;
}

vec3 ambient(vec3 intensity) {
	//if (has_texture) {
		return intensity * vec3(texture(sampler2D(textures[pc.imgIdx], diffuse_sampler), uv));
	//}
	//return intensity * fs_in.color;
}

vec3 diffuse(vec3 intensity, vec3 receive_direction) {
	float impact = max(dot(normal, receive_direction), 0.0f);
	//if (has_texture) {
		return intensity * impact * vec3(texture(sampler2D(textures[pc.imgIdx], diffuse_sampler), uv));
	//}
	//return intensity * impact * fs_in.color;
}

void main() {
	//color = vec4(vec3(calculate_shadows(light_space_position) * texture(diffuse_texture, uv)), 1.0);
	vec3 direction_lighting_result = vec3(0);
	float shadowed = calculate_shadows(light_space_position);

	// Directional Lighting
	{
		vec3 receive_direction = normalize(-directional_light_direction);
		//dir_result = calculate_shadows(fs_in.light_space_position) * vec3(1.0);
		direction_lighting_result = ambient(ambient_intensity) +
		                            (shadowed * diffuse(diffuse_intensity, receive_direction));
		//diffuse(diffuse_intensity, recv_dir);
		//specular(dir.specular.xyz, recv_dir, view_dir);
	}

	color = vec4(direction_lighting_result, 1.0);
}
