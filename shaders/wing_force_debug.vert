#version 460

layout(location = 0) in vec3 in_Position;

layout(location = 1) out vec3 v_ViewPos;

layout(std140, binding = 0) uniform FrameUBO {
	mat4 projection;
	mat4 view;
} u_Frame;

layout(std140, binding = 1) uniform ModelUBO {
	mat4 transform;
} u_Model;

void main() {
	v_ViewPos = (u_Frame.view * u_Model.transform * vec4(in_Position, 1.0)).xyz;
	gl_Position = u_Frame.projection * vec4(v_ViewPos, 1.0);
}
