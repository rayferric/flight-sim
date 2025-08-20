#include "fps_camera.hpp"

void fps_camera::update_fps_pose_from_input(window &window, float dt) {
	// if esc pressed, release cursor
	if (window.is_glfw_key_down(GLFW_KEY_ESCAPE)) {
		captured_cursor = false;
	}

	// if clicked anywhere in the window, capture cursor
	if (glfwGetMouseButton(window.glfw_window, GLFW_MOUSE_BUTTON_LEFT) ==
	    GLFW_PRESS) {
		captured_cursor = true;
	}

	// set cursor mode based on captured state
	glfwSetInputMode(
	    window.glfw_window,
	    GLFW_CURSOR,
	    captured_cursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
	);

	// calculate mouse delta
	double mouse_x, mouse_y;
	glfwGetCursorPos(window.glfw_window, &mouse_x, &mouse_y);
	if (last_mouse_x == 0 && last_mouse_y == 0) {
		last_mouse_x = static_cast<float>(mouse_x);
		last_mouse_y = static_cast<float>(mouse_y);
	}
	float mouse_dx = static_cast<float>(mouse_x) - last_mouse_x;
	float mouse_dy = static_cast<float>(mouse_y) - last_mouse_y;
	last_mouse_x   = static_cast<float>(mouse_x);
	last_mouse_y   = static_cast<float>(mouse_y);

	// smooth mouse delta
	constexpr float smoothing = 0.7f;
	mouse_dx      = mouse_dx * (1.0f - smoothing) + last_mouse_dx * smoothing;
	mouse_dy      = mouse_dy * (1.0f - smoothing) + last_mouse_dy * smoothing;
	last_mouse_dx = mouse_dx;
	last_mouse_dy = mouse_dy;

	if (mouse_fix_iters_to_init > 0) {
		// skip first few iterations to allow for mouse deltas to stabilize
		mouse_fix_iters_to_init--;
		return;
	}

	// update rot if captured
	if (captured_cursor) {
		// remap sensitivity from rots per screen to deg/px
		// > get monitor width
		int monitor_width, monitor_height;
		glfwGetMonitorWorkarea(
		    glfwGetPrimaryMonitor(),
		    nullptr,
		    nullptr,
		    &monitor_width,
		    &monitor_height
		);
		float unit_deg_per_px = 360.0f / monitor_width;
		float movement_scale  = unit_deg_per_px * sensitivity;

		// compute current pitch/yaw
		float &pitch  = rpy_deg.y;
		float &yaw    = rpy_deg.z;
		pitch        += mouse_dy * movement_scale;
		yaw          -= mouse_dx * movement_scale;
		pitch         = std::clamp(pitch, -89.0f, 89.0f);

		// read WSADEQ input
		constexpr glm::vec3 forward(1.0f, 0.0f, 0.0f);
		constexpr glm::vec3 left(0.0f, 1.0f, 0.0f);
		constexpr glm::vec3 up(0.0f, 0.0f, 1.0f);
		glm::vec3           move_offset = glm::vec3(0.0f);
		if (window.is_glfw_key_down(GLFW_KEY_W)) {
			move_offset += forward * dt * movement_speed;
		}
		if (window.is_glfw_key_down(GLFW_KEY_S)) {
			move_offset -= forward * dt * movement_speed;
		}
		if (window.is_glfw_key_down(GLFW_KEY_A)) {
			move_offset += left * dt * movement_speed;
		}
		if (window.is_glfw_key_down(GLFW_KEY_D)) {
			move_offset -= left * dt * movement_speed;
		}
		if (window.is_glfw_key_down(GLFW_KEY_E)) {
			move_offset += up * dt * movement_speed;
		}
		if (window.is_glfw_key_down(GLFW_KEY_Q)) {
			move_offset -= up * dt * movement_speed;
		}
		if (window.is_glfw_key_down(GLFW_KEY_LEFT_SHIFT)) {
			move_offset *= movement_boost;
		}
		if (window.is_glfw_key_down(GLFW_KEY_LEFT_CONTROL)) {
			move_offset *= movement_slow;
		}

		// rotate move offset using rpy
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), left);
		rot         = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), up) * rot;
		move_offset = glm::vec3(rot * glm::vec4(move_offset, 0.0f));
		pos_flu += move_offset;

		set_pose(pos_flu, rpy_deg);
	}
}
