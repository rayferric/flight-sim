#pragma once

#include "../pch.hpp"

#include "../gfx/window.hpp"
#include "entity/basic_camera.hpp"

class follow_camera : public basic_camera {
public:
	void update_pose_from_follow_target(
	    window &window, float dt, glm::vec3 target_pos
	);

private:
	const float follow_radius  = 25.0f;
	const float max_speed      = 500.0f; // units per second
	const float speed_exponent = 0.02f;
};
