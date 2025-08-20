#version 460

layout(location = 0) out vec4 out_Color;

layout(location = 1) in vec3 v_ViewPos;

layout(std140, binding = 2) uniform WingForceDebugColor {
	vec3 color;
} u_WingForceDebugColor;

void main() {
	vec3 color = u_WingForceDebugColor.color;
	out_Color = vec4(color, 1.0);
}
