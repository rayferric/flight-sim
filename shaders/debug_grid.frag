#version 460

layout(location = 0) out vec4 out_Color;

layout(location = 0) in vec3 v_Color;
layout(location = 1) in vec3 v_ViewPos;

layout(std140, binding = 2) uniform GridSettingsUBO {
	float renderDistance;
} u_GridSettings;

void main() {
	float dist = length(v_ViewPos);
	if (dist > u_GridSettings.renderDistance) {
		discard;
	} else {
		float falloff = 1.0 - (dist / u_GridSettings.renderDistance);
		falloff = clamp(falloff, 0.0, 1.0);
		vec3 color = v_Color;
		out_Color = vec4(color, falloff);
	}
}
