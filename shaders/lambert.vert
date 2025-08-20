#version 460

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec3 in_Tangent;

layout(location = 0) out vec2 v_TexCoord;
layout(location = 1) out mat3 v_TBN;
layout(location = 4) out vec3 v_Normal;

layout(std140, binding = 0) uniform FrameUBO {
	mat4 projection;
	mat4 view;
} u_Frame;

layout(std140, binding = 1) uniform ModelUBO {
	mat4 transform;
} u_Model;

void main() {
	v_TexCoord = in_TexCoord;

	vec3 T = normalize(mat3(u_Model.transform) * in_Tangent);
   	vec3 N = normalize(mat3(u_Model.transform) * in_Normal);
	vec3 B = normalize(cross(T, N));
	
	v_TBN = transpose(mat3(T, B, N));
	v_Normal = normalize(mat3(u_Model.transform) * in_Normal);

	gl_Position = u_Frame.projection * u_Frame.view * u_Model.transform * vec4(in_Position, 1.0);
}
