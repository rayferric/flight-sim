#pragma once

#include "../pch.hpp"

#include "../dynamics/wing.hpp"
#include "../gfx/colored_mesh.hpp"
#include "../gfx/mesh.hpp"
#include "../gfx/shader.hpp"
#include "../gfx/uniform_buffer.hpp"
#include "../gfx/window.hpp"
#include "transform.hpp"

struct jet_force_vec {
	glm::vec3 force  = glm::vec3(0.0f);
	glm::vec3 origin = glm::vec3(0.0f);
};

class jet {
public:
	void init(
	    const std::filesystem::path &mesh_path,
	    const std::filesystem::path &shader_vert_path,
	    const std::filesystem::path &shader_frag_path,
	    const std::filesystem::path &cl_curve_path,
	    const std::filesystem::path &wing_force_debug_shader_vert_path,
	    const std::filesystem::path &wing_force_debug_shader_frag_path
	);

	void      draw(bool wing_force_debug = true);
	glm::vec3 get_center_of_mass();
	glm::quat get_quat();
	glm::vec3 get_rpy();
	void      update_physics_from_input(window &window, float dt);

protected:
	uniform_buffer model_ubo;
	mesh           visual_mesh;
	shader         shader_;

	const float throttle_level_rate_of_change = 0.5f; // units/s

	const float empty_mass     = 22500.0f;  // kg
	const float max_fuel       = 12100.0f;  // kg
	const float max_thrust_dry = 153000.0f; // TODO FIXME: added 400k extra
	                                        // thrust to make it maneuverable
	const float max_thrust_wet = 245000.0f; // N

	glm::vec3 pos            = glm::vec3(0.0f);                   // m
	glm::quat rot            = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // w, x, y, z
	glm::vec3 vel            = glm::vec3(100.0f, 0, 0);           // m/s
	glm::vec3 ang_vel        = glm::vec3(0.0f); // r-vec, rad/s
	float     throttle_level = 0.0f; // (0, 1), go over 1.0f for afterburner
	float     fuel_level     = 1.0f; // (0, 1), 1.0f is full tank
	bool      flaps_down     = false;
	bool      flaps_down_key_just_pressed  = false;
	bool      afterburner_on               = false;
	bool      afterburner_key_just_pressed = false;

	// wing parameters (look for wing params in init())
	const glm::vec3 center_of_mass              = {-13.0f, 0.0f, -0.3f};
	const glm::vec3 center_of_thrust            = {-20.0f, 0.0f, -0.8f};
	const glm::vec3 left_wing_root_pos          = {-14.4f, 2.25f, 0.0f};
	const glm::vec3 right_wing_root_pos         = {-14.4f, -2.25f, 0.0f};
	const glm::vec3 left_h_stabilizer_root_pos  = {-19.0f, 2.2f, -0.9f};
	const glm::vec3 right_h_stabilizer_root_pos = {-19.0f, -2.2f, -0.9f};
	const glm::vec3 left_v_stabilizer_root_pos  = {-17.2f, 2.2f, 0.0f};
	const glm::vec3 right_v_stabilizer_root_pos = {-17.2f, -2.2f, 0.0f};
	const glm::vec3 left_canard_root_pos        = {-9.4f, 1.9f, 0.1f};
	const glm::vec3 right_canard_root_pos       = {-9.4f, -1.9f, 0.1f};
	const float     thrust_incidence_deg        = 2.5f;
	const float     wing_incidence_deg          = 4.0f;
	const float     h_stabilizer_incidence_deg  = 2.5f;
	const float     v_stabilizer_vshape_deg     = 0.0f; // perfectly vertical
	const float     canard_incidence_deg        = 2.5f;
	wing            main_wing;
	wing            h_stabilizer;
	wing            v_stabilizer;
	wing            canard;

	// wing debug
	colored_mesh               wing_force_debug_mesh;
	uniform_buffer             wing_force_debug_model_ubo;
	uniform_buffer             wing_force_debug_color_ubo;
	shader                     wing_force_debug_shader;
	std::vector<jet_force_vec> debug_wing_forces;

	// input smoothing
	float pitch_down_level_smooth  = 0.0f;
	float roll_right_level_smooth  = 0.0f;
	float rudder_left_level_smooth = 0.0f;

	int log_counter = 0;

	void update_ubo();
};
