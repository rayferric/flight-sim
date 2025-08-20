#include "basic_camera.hpp"

void basic_camera::bind() {
	view_ubo.bind(0);
}

void basic_camera::set_fov(float major_fov_deg) {
	this->major_fov_deg = major_fov_deg;
	update_ubo();
}

void basic_camera::set_res(uint32_t width, uint32_t height) {
	this->width  = width;
	this->height = height;
	update_ubo();
}

void basic_camera::set_pose(const glm::vec3 &pos, const glm::vec3 &rpy) {
	transform::set_pose(pos, rpy);
	update_ubo();
}

void basic_camera::update_ubo() {
	// proj matrix
	float fov_y_deg = major_fov_deg;
	if (width > height) {
		// scale down vertical FOV when screen is horizontal
		float tan_half_fov  = std::tan(glm::radians(major_fov_deg * 0.5f));
		tan_half_fov       *= (float)height / (float)width;
		fov_y_deg           = glm::degrees(2.0f * std::atan(tan_half_fov));
	}
	glm::mat4 gl_proj = glm::perspective(
	    glm::radians(fov_y_deg), (float)width / (float)height, 0.1f, 1000.0f
	);
	constexpr glm::mat3 flu_to_gl = {
	    {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}
	};
	glm::mat4 gl_proj_flu = gl_proj * glm::mat4(flu_to_gl);
	// ^ = (flu_to_gl * flu_proj) * FLU_POS
	//   = (flu_to_gl * gl_to_flu * gl_proj * flu_to_gl) * FLU_POS
	//   = (gl_proj * flu_to_gl) * FLU_POS

	// view matrix
	glm::mat4 view_mat = glm::inverse(transform::calc_mat4_flu_space());

	// ubo update
	view_ubo.update(gl_proj_flu, view_mat);
}
