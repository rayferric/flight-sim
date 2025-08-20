#version 460

layout(location = 0) in vec3 in_Position;

layout(location = 1) out vec2 vTexCoord;

void main() {
	vTexCoord = in_Position.xy * 0.5 + 0.5;
	gl_Position = vec4(in_Position, 1.0);
}
