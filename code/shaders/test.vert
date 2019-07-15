#version 420

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec3 vertex_uv;

layout(location = 0) out vec3 fragment_color;
layout(location = 1) out vec3 fragment_uv;

uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

void main() {
    gl_Position = vec4(vertex_position, 1.0) * ubo.model * ubo.view * ubo.projection;
    //gl_Position = vec4(vertex_position, 0.0, 1.0);
    fragment_color = vertex_color;
    fragment_uv = vertex_uv;
}
