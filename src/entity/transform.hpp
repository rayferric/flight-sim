#pragma once

#include "../pch.hpp"

class transform {
public:
	virtual ~transform() = default;

	virtual void set_pose(const glm::vec3 &pos_flu, const glm::vec3 &rpy_deg);

	glm::vec3 get_pos_flu() const;
	glm::vec3 get_rpy_deg() const;

protected:
	glm::vec3 pos_flu = glm::vec3(0.0f), rpy_deg = glm::vec3(0.0f);

	glm::mat4 calc_mat4_flu_space() const;
};
