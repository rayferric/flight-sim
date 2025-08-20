#version 460

layout(location = 0) out vec4 out_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in mat3 v_TBN;
layout(location = 4) in vec3 v_Normal;

void main() {
	// vec3 normalMap = vec3(0.5, 0.5, 1.0);
	// vec3 N = normalize((normalMap * 2.0 - 1.0) * v_TBN);
	vec3 N = normalize(v_Normal);

	vec3 L = normalize(vec3(1.0, 1.0, 1.0));

	vec3 color = vec3(max(dot(N, L), 0.5)) * vec3(0.9, 0.95, 1.0);
	out_Color = vec4(color, 1.0);
}
