#include "jet.hpp"

#include "../dynamics/wing_3d_helper.hpp"

void jet::init(
    const std::filesystem::path &mesh_path,
    const std::filesystem::path &shader_vert_path,
    const std::filesystem::path &shader_frag_path,
    const std::filesystem::path &cl_curve_path,
    const std::filesystem::path &wing_force_debug_shader_vert_path,
    const std::filesystem::path &wing_force_debug_shader_frag_path
) {
	mesh.load_from_file(mesh_path);
	shader_.compile_from_file(shader_vert_path, shader_frag_path);
	update_ubo();

	// Su-34 wing shape approximation
	curve cl_vs_aoa_curve;
	cl_vs_aoa_curve.load_from_file(cl_curve_path);
	airfoil airfoil(cl_vs_aoa_curve);
	main_wing = wing(
	    airfoil,
	    {
	        {
	            .span        = 3.0f,
	            .chord       = 4.0f,
	            .has_aileron = true,
	            .has_slat    = true,
	        },
	        {
	            .span            = 1.5f,
	            .chord           = 2.8f,
	            .chordwise_shift = 1.4f,
	            .has_slat        = true,
	        },
	        {
	            .span            = 0.7f,
	            .chord           = 2.1f,
	            .chordwise_shift = 0.4f,
	        },
	    }
	);
	h_stabilizer = wing(
	    airfoil,
	    {
	        {
	            .span  = 2.3f,
	            .chord = 2.3f,
	        },
	    }
	);
	v_stabilizer = wing(
	    airfoil,
	    {
	        {
	            .span        = 2.0f,
	            .chord       = 2.5f,
	            .has_aileron = true,
	        },
	        {
	            .span            = 1.1f,
	            .chord           = 1.4f,
	            .chordwise_shift = 0.8f,
	        },
	    }
	);
	canard = wing(
	    airfoil,
	    {
	        {
	            .span  = 1.4f,
	            .chord = 1.0f,
	        },
	    }
	);

	// wing debug
	std::vector<colored_mesh::vertex> verts;
	colored_mesh::vertex              v1, v2;
	v1.pos[0] = v1.pos[1] = v1.pos[2] = 0.0f;
	v2.pos[0] = v2.pos[1] = v2.pos[2] = 0.0f;
	v2.pos[2]                         = 10.0f; // 1m vertical line
	verts.push_back(v1);
	verts.push_back(v2);
	wing_force_debug_mesh.load(verts);
	wing_force_debug_shader.compile_from_file(
	    wing_force_debug_shader_vert_path, wing_force_debug_shader_frag_path
	);
	wing_force_debug_color_ubo.update(glm::vec3(0.0f, 1.0f, 0.0f));
}

void jet::draw() {
	shader_.bind();
	model_ubo.bind(1);
	mesh.draw();

	// wing debug
	glLineWidth(3.0f);
	wing_force_debug_shader.bind();
	wing_force_debug_model_ubo.bind(1);
	wing_force_debug_color_ubo.bind(2);
	for (const auto &f : debug_wing_forces) {
		glm::vec3 dir       = glm::normalize(f.force);
		float     magnitude = glm::length(f.force) * 0.0001f;

		// Find rotation from (0,0,1) to dir
		glm::vec3 default_dir = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 cross_axis  = glm::normalize(glm::cross(default_dir, dir));
		glm::quat rotation =
		    glm::angleAxis(glm::acos(glm::dot(default_dir, dir)), cross_axis);

		glm::mat4 model  = glm::mat4(1.0f);
		model            = glm::translate(model, f.origin);
		model           *= glm::mat4_cast(rotation);
		model            = glm::scale(model, glm::vec3(1.0f, 1.0f, magnitude));

		glm::mat4 parent_model =
		    glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot);
		model = parent_model * model;

		const glm::vec3 green(0.0f, 1.0f, 0.0f);
		const glm::vec3 red(1.0f, 0.0f, 0.0f);
		float           mix_fac = std::clamp(magnitude / 1.0f, 0.0f, 1.0f);
		glm::vec3       color   = glm::mix(green, red, mix_fac);
		color                   = glm::pow(color, glm::vec3(1.0f / 2.2f));

		wing_force_debug_model_ubo.update(model);
		wing_force_debug_color_ubo.update(color);
		wing_force_debug_mesh.draw();
	}
}

glm::vec3 jet::get_center_of_mass() {
	glm::vec3 pos  = this->pos;
	pos           += rot * glm::vec3(center_of_mass);
	return pos;
}

glm::quat jet::get_quat() {
	return rot;
}

glm::vec3 jet::get_rpy() {
	glm::vec3 rpy = glm::eulerAngles(rot);
	return glm::vec3(
	    glm::degrees(rpy.x), // roll
	    glm::degrees(rpy.y), // pitch
	    glm::degrees(rpy.z)  // yaw
	);
}

void jet::update_physics_from_input(window &window, float dt) {
	rot = glm::normalize(rot); // ensure quaternion is normalized

	// process input
	if (window.is_glfw_key_down(GLFW_KEY_LEFT_SHIFT)) {
		throttle_level += throttle_level_rate_of_change * dt;
		throttle_level  = glm::min(throttle_level, 1.0f);
	}
	if (window.is_glfw_key_down(GLFW_KEY_LEFT_CONTROL)) {
		throttle_level -= throttle_level_rate_of_change * dt;
		throttle_level  = glm::max(throttle_level, 0.0f);
	}
	if (window.is_glfw_key_down(GLFW_KEY_Z)) {
		throttle_level = 1.0f;
	} else if (window.is_glfw_key_down(GLFW_KEY_X)) {
		throttle_level = 0.0f;
	}
	float pitch_down_rate =
	    static_cast<int>(window.is_glfw_key_down(GLFW_KEY_W)) -
	    window.is_glfw_key_down(GLFW_KEY_S);
	float roll_right_rate =
	    static_cast<int>(window.is_glfw_key_down(GLFW_KEY_D)) -
	    window.is_glfw_key_down(GLFW_KEY_A);
	float rudder_left_rate =
	    static_cast<int>(window.is_glfw_key_down(GLFW_KEY_Q)) -
	    window.is_glfw_key_down(GLFW_KEY_E);

	const glm::vec3 forward_vec = glm::vec3(1.0f, 0.0f, 0.0f);
	const glm::vec3 up_vec      = glm::vec3(0.0f, 0.0f, 1.0f);

	// all forces are in local space
	std::vector<jet_force_vec> forces;

	// thrust
	float         thrust = throttle_level * max_thrust_dry; // N
	jet_force_vec thrust_force;
	glm::quat     thrust_rot = glm::angleAxis(
        glm::radians(-0.0f), glm::vec3(0.0f, 1.0f, 0.0f)
    ); // TODO FIXME
	thrust_force.force   = thrust_rot * forward_vec;
	thrust_force.force  *= thrust;
	thrust_force.origin  = center_of_thrust;
	forces.push_back(thrust_force);

	// air velocity in local airplane space
	glm::vec3 local_vel     = glm::inverse(rot) * vel;
	glm::vec3 local_ang_vel = glm::inverse(rot) * ang_vel;
	glm::quat left_wing_rot = glm::angleAxis(
	    glm::radians(-wing_incidence_deg), glm::vec3(0.0f, 1.0f, 0.0f)
	);
	wing_forces_3d wing_forces = calc_wing_forces_3d(
	    main_wing,
	    local_vel,
	    local_ang_vel,
	    center_of_mass,
	    left_wing_root_pos,
	    left_wing_rot,
	    false,
	    pitch_down_rate * 20.0f + roll_right_rate * 20.0f
	);
	for (const auto &f : gather_wing_forces_3d(wing_forces)) {
		forces.push_back({f.force, f.origin});
	}
	glm::quat right_wing_rot = glm::angleAxis(
	    glm::radians(-wing_incidence_deg), glm::vec3(0.0f, 1.0f, 0.0f)
	);
	wing_forces_3d wing_forces2 = calc_wing_forces_3d(
	    main_wing,
	    local_vel,
	    local_ang_vel,
	    center_of_mass,
	    right_wing_root_pos,
	    right_wing_rot,
	    true,
	    pitch_down_rate * 20.0f - roll_right_rate * 20.0f
	);
	for (const auto &f : gather_wing_forces_3d(wing_forces2)) {
		forces.push_back({f.force, f.origin});
	}

	// save forces for debug
	debug_wing_forces = forces;

	// combine forces -> calc acceleration

	// total force
	glm::vec3 total_force = glm::vec3(0.0f);
	for (const auto &f : forces) {
		total_force += f.force;
	}
	// total torque
	glm::vec3 total_torque = glm::vec3(0.0f);
	for (const auto &f : forces) {
		glm::vec3 r       = f.origin - center_of_mass;
		glm::vec3 torque  = glm::cross(r, f.force);
		total_torque     += torque;
	}
	// mass
	float total_mass = empty_mass + (fuel_level * max_fuel); // kg
	// inertia tensor
	// placeholder: solid ball, r = 2 m, m = 30000 kg
	glm::mat3 inertia_tensor = glm::mat3(
	    48000.0f,
	    0.0f,
	    0.0f, // Ixx
	    0.0f,
	    48000.0f,
	    0.0f, // Iyy
	    0.0f,
	    0.0f,
	    48000.0f // Izz
	);
	// accelerations
	glm::vec3 accel = total_force / total_mass; // m/s^2
	glm::vec3 ang_accel =
	    glm::inverse(inertia_tensor) * total_torque; // rvec, rad/s^2
	// transform acceleration to global frame
	accel     = rot * accel;
	ang_accel = rot * ang_accel;
	// apply gravity
	// accel.z -= 9.81f;

	// integrate movement
	vel     += accel * dt;     // m/s
	pos     += vel * dt;       // m
	ang_vel += ang_accel * dt; // rvec, rad/s
	if (glm::length(ang_vel) > 1e-6f) {
		glm::quat d_rot =
		    glm::angleAxis(glm::length(ang_vel) * dt, glm::normalize(ang_vel));
		// ... but we need to rotate around the center of mass
		// idk how, but this works:
		glm::vec3 com_offset  = rot * glm::vec3(center_of_mass);
		pos                  -= d_rot * com_offset - com_offset;
		rot                   = glm::normalize(d_rot * rot);
	}
	update_ubo();
}

void jet::update_ubo() {
	glm::mat4 model_mat =
	    glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot);
	model_ubo.update(model_mat);
}
