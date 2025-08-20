#pragma once

#include "../pch.hpp"

#include "../gfx/uniform_buffer.hpp"
#include "transform.hpp"

class basic_camera : public transform {
public:
	void bind();

	void set_fov(float major_fov_deg);
	void set_res(uint32_t width, uint32_t height);
	void set_pose(const glm::vec3 &pos_flu, const glm::vec3 &rpy_deg) override;

protected:
	uniform_buffer view_ubo;

	float    major_fov_deg = 90.0f;
	uint32_t width = 400, height = 300;

	void update_ubo();
};
