#include "follow_camera.hpp"
#include <cmath>
#include <glm/trigonometric.hpp>

void follow_camera::update_pose_from_follow_target(
    window &window, float dt, glm::vec3 target_pos
) {
	glm::vec3 target_to_cam = pos_flu - target_pos;
	glm::vec3 dst_cam_pos =
	    target_pos + glm::normalize(target_to_cam) * follow_radius;

	glm::vec3 move_offset     = dst_cam_pos - pos_flu;
	float     move_offset_len = glm::length(move_offset);
	float     cam_speed =
	    (1 - glm::exp(-move_offset_len * speed_exponent)) * max_speed;

	pos_flu += glm::normalize(move_offset) * cam_speed * dt;

	// look at target

	glm::vec3 forward = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 left    = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 up      = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec3 look_at_dir = glm::normalize(target_pos - pos_flu);

	glm::vec3 look_at_dir_right = glm::normalize(glm::cross(look_at_dir, up));
	glm::vec3 look_at_dir_horizon =
	    glm::normalize(glm::cross(up, look_at_dir_right));

	float pitch = glm::degrees(
	    glm::asin(
	        glm::distance(
	            glm::cross(look_at_dir_horizon, look_at_dir), look_at_dir_right
	        ) -
	        1.0f
	    )
	);

	float yaw =
	    glm::degrees(glm::atan(look_at_dir_horizon.y, look_at_dir_horizon.x));

	if (std::isnan(pitch) || std::isinf(pitch) || std::abs(pitch) > 89.0f ||
	    std::isnan(yaw) || std::isinf(yaw) || std::abs(yaw) > 180.0f) {
		pitch = rpy_deg.y;
		yaw   = rpy_deg.z;
	}

	set_pose(pos_flu, glm::vec3(0.0f, pitch, yaw));
}
