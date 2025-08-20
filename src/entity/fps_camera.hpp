#pragma once

#include "../pch.hpp"

#include "../gfx/window.hpp"
#include "entity/basic_camera.hpp"

class fps_camera : public basic_camera {
public:
	void update_fps_pose_from_input(window &window, float dt);

private:
	const float movement_speed = 10.0f; // units per second
	const float movement_boost = 3.0f;
	const float movement_slow  = 0.1f;
	const float sensitivity    = 0.25f;
	// ^ number of full rots per full monitor width of mouse movement

	uint32_t mouse_fix_iters_to_init = 2;
	float    last_mouse_x            = 0.0f;
	float    last_mouse_y            = 0.0f;
	float    last_mouse_dx           = 0.0f;
	float    last_mouse_dy           = 0.0f;

	bool captured_cursor_on_first_iter = false;
	bool captured_cursor               = true;
};
