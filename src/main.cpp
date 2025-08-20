#include "pch.hpp"

#include "entity/basic_camera.hpp"
#include "entity/debug_grid.hpp"
#include "entity/follow_camera.hpp"
#include "entity/fps_camera.hpp"
#include "entity/jet.hpp"
#include "gfx/mesh.hpp"
#include "gfx/shader.hpp"
#include "gfx/uniform_buffer.hpp"
#include "gfx/window.hpp"

int main() {
	window window;
	window.open(800, 600, "Flight Sim");

	// follow_camera cam;
	basic_camera cam;
	cam.set_pose(glm::vec3(-40.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f));

	jet jet;
	jet.init(
	    "../meshes/su34.obj",
	    "../shaders/lambert.vert",
	    "../shaders/lambert.frag",
	    "../curves/su34_lift_aoa.txt",
	    "../shaders/wing_force_debug.vert",
	    "../shaders/wing_force_debug.frag"
	);

	debug_grid grid;
	grid.init("../shaders/debug_grid.vert", "../shaders/debug_grid.frag");

	// init on_draw
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.08, 0.08, 0.1, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	cam.bind();

	// init on_update
	std::chrono::time_point last_update_time = std::chrono::steady_clock::now();
	float                   elapsed_ms       = 0.0f;

	// clang-format off
	window.run_loop({
		.on_draw = [&]() {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			jet.draw();
			grid.draw();
		},
		.on_update = [&]() {
			std::chrono::time_point now = std::chrono::steady_clock::now();
			float dt = std::chrono::duration<float>(
				now - last_update_time
			).count();
			last_update_time = now;

			// cam.update_fps_pose_from_input(window, dt);
			// cam.update_pose_from_follow_target(window, dt, jet.get_center_of_mass());
			glm::vec3 com = jet.get_center_of_mass();
			glm::vec3 rpy = jet.get_rpy();
			glm::quat rot = jet.get_quat();
			cam.set_pose(
			    com + rot * glm::vec3(-30.0f, 0.0f, 10.0f),
			    glm::vec3(0.0f, rpy.y, rpy.z)
			);
			grid.update_tiling_from_view_pos(cam.get_pos_flu());
			jet.update_physics_from_input(window, dt);
		},
	    .on_resize = [&](uint32_t w, uint32_t h) {
			glViewport(0, 0, w, h);
			cam.set_res(w, h);
		}
	});
	// clang-format on

	return 0;
}
