#pragma once

#include "../pch.hpp"

#include "wing.hpp"

struct wing_force_vec_3d {
	glm::vec3 force;
	glm::vec3 origin;
};

struct wing_forces_3d {
	std::vector<wing_force_vec_3d> sectional_lift;
	std::vector<wing_force_vec_3d> sectional_drag;
	wing_force_vec_3d              induced_drag;
};

inline glm::vec3 wing_local_velocity(
    glm::vec3 point, glm::vec3 vel, glm::vec3 ang_vel, glm::vec3 rot_origin
) {
	return vel + glm::cross(ang_vel, point - rot_origin);
}

// COORDINATE SYSTEM DETAILS:
// Assumes left wing: Extends along +Y axis and produces lift towards Z+
// when moving towards +X. In order to invert extension direction to -Y, set
// is_right_wing = true. If you try to invert using wing_mount_rot, the
// computed angle of attack will be negated!
// wing_mount_pos/rot should transform from the described coordinate
// convention to the same space as linear/angular_velocity and
// origin_of_rotation
struct wing_speed_aoa_and_move_dirs {
	std::vector<wing_speed_aoa> speed_aoa;
	std::vector<glm::vec3>      move_dirs;
};
inline wing_speed_aoa_and_move_dirs wing_sectional_speed_aoa(
    const wing &wing,
    glm::vec3   linear_velocity,
    glm::vec3   angular_velocity,
    glm::vec3   origin_of_rotation,
    glm::vec3   wing_mount_pos,
    glm::quat   wing_mount_rot,
    bool        is_right_wing = false
) {
	wing_mount_rot = glm::normalize(wing_mount_rot);

	const glm::vec3 forward_vec = glm::vec3(1.0f, 0.0f, 0.0f);
	const glm::vec3 left_vec    = glm::vec3(0.0f, 1.0f, 0.0f);
	const glm::vec3 up_vec      = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec3 wing_forward_dir = wing_mount_rot * forward_vec;
	glm::vec3 wing_left_dir    = wing_mount_rot * left_vec;
	glm::vec3 wing_up_dir      = wing_mount_rot * up_vec;

	std::vector<wing_speed_aoa> result(wing.sections.size());
	std::vector<glm::vec3>      move_dirs(wing.sections.size());
	float                       cumulative_span = 0.0f;
	float                       chordwise_shift = 0.0f;
	for (size_t i = 0; i < wing.sections.size(); i++) {
		const wing_section &section = wing.sections[i];

		// section center in same ref frame as rotation origin
		glm::vec3 section_center(
		    -chordwise_shift, cumulative_span + section.span * 0.5f, 0.0f
		);
		if (is_right_wing) {
			section_center.y = -section_center.y;
		}
		section_center = wing_mount_rot * section_center + wing_mount_pos;

		// airspeed
		glm::vec3 section_vel = wing_local_velocity(
		    section_center,
		    linear_velocity,
		    angular_velocity,
		    origin_of_rotation
		);
		move_dirs[i] = glm::normalize(section_vel);
		glm::vec3 vel_in_aerodynamic_plane =
		    section_vel - glm::dot(section_vel, wing_left_dir) * wing_left_dir;
		float airspeed = glm::length(vel_in_aerodynamic_plane);
		// aoa
		float cosine_forward = glm::dot(
		    wing_forward_dir, glm::normalize(vel_in_aerodynamic_plane)
		);
		float cosine_up =
		    glm::dot(wing_up_dir, glm::normalize(vel_in_aerodynamic_plane));
		cosine_forward = std::isnan(cosine_forward)
		                   ? 1.0
		                   : std::clamp(cosine_forward, -1.0f, 1.0f);
		cosine_up =
		    std::isnan(cosine_up) ? 0.0 : std::clamp(cosine_up, -1.0f, 1.0f);
		float aoa = -glm::degrees(std::atan2(cosine_up, cosine_forward));
		// ^ minus since when wing is moving upwards (positive atan2 angle), the
		// air hits from above (need negative aoa)

		result[i] = {airspeed, aoa};

		cumulative_span += section.span;
		chordwise_shift += section.chordwise_shift;
	}

	return {result, move_dirs};
}

inline glm::vec3 safe_normalize(const glm::vec3 &v) {
	if (glm::length(v) > 1e-4f) {
		return glm::normalize(v);
	} else {
		return glm::vec3(0.0f);
	}
}

inline wing_force_vec_3d map_wing_force_to_3d(
    const wing_force_vec &force,
    glm::vec3             wing_mount_pos,
    glm::quat             wing_mount_rot,
    bool                  is_drag,
    glm::vec3             move_dir,
    bool                  is_right_wing
) {
	const glm::vec3 left_vec      = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3       wing_left_dir = wing_mount_rot * left_vec;

	move_dir = safe_normalize(move_dir);
	glm::vec3 aerodynamic_plane_move_dir =
	    move_dir - glm::dot(move_dir, wing_left_dir) * wing_left_dir;
	glm::vec3 lift_dir =
	    safe_normalize(glm::cross(aerodynamic_plane_move_dir, wing_left_dir));
	glm::vec3 drag_dir = -move_dir;

	wing_force_vec_3d result;
	result.origin = wing_mount_rot * glm::vec3(
	                                     -force.origin_chordwise,
	                                     is_right_wing ? -force.origin_spanwise
	                                                   : force.origin_spanwise,
	                                     0.0f
	                                 ) +
	                wing_mount_pos;
	result.force = force.force * (is_drag ? drag_dir : lift_dir);

	return result;
}

inline wing_forces_3d map_wing_forces_to_3d(
    const wing_forces            &forces,
    glm::vec3                     wing_mount_pos,
    glm::quat                     wing_mount_rot,
    const std::vector<glm::vec3> &section_move_dirs,
    const glm::vec3               main_move_dir,
    bool                          is_right_wing
) {
	wing_forces_3d result;
	result.sectional_lift.reserve(forces.sectional_lift.size());
	result.sectional_drag.reserve(forces.sectional_lift.size());

	for (size_t i = 0; i < forces.sectional_lift.size(); i++) {
		result.sectional_lift.push_back(map_wing_force_to_3d(
		    forces.sectional_lift[i],
		    wing_mount_pos,
		    wing_mount_rot,
		    false,
		    section_move_dirs[i],
		    is_right_wing
		));
		result.sectional_drag.push_back(map_wing_force_to_3d(
		    forces.sectional_drag[i],
		    wing_mount_pos,
		    wing_mount_rot,
		    true,
		    section_move_dirs[i],
		    is_right_wing
		));
	}

	result.induced_drag = map_wing_force_to_3d(
	    forces.induced_drag,
	    wing_mount_pos,
	    wing_mount_rot,
	    true,
	    main_move_dir,
	    is_right_wing
	);
	return result;
}

inline wing_forces_3d calc_wing_forces_3d(
    wing     &wing,
    glm::vec3 linear_velocity,
    glm::vec3 angular_velocity,
    glm::vec3 origin_of_rotation,
    glm::vec3 wing_mount_pos,
    glm::quat wing_mount_rot,
    bool      is_right_wing = false,
    float     aileron_deg   = 0.0f,
    float     flap_deg      = 0.0f,
    float     slat_deg      = 0.0f,
    float     air_density   = 1.225f
) {
	wing_mount_rot = glm::normalize(wing_mount_rot);

	auto [speed_aoa, move_dirs] = wing_sectional_speed_aoa(
	    wing,
	    linear_velocity,
	    angular_velocity,
	    origin_of_rotation,
	    wing_mount_pos,
	    wing_mount_rot,
	    is_right_wing
	);
	wing_forces forces = wing.calc_forces(
	    speed_aoa, aileron_deg, flap_deg, slat_deg, air_density
	);

	// weighted mean move dir
	glm::vec3 mean_move_dir(0.0f);
	float     total_area = 0.0f;
	for (size_t i = 0; i < move_dirs.size(); ++i) {
		float area     = wing.sections[i].span * wing.sections[i].chord;
		mean_move_dir += move_dirs[i] * area;
		total_area    += area;
	}
	mean_move_dir /= total_area;

	return map_wing_forces_to_3d(
	    forces,
	    wing_mount_pos,
	    wing_mount_rot,
	    move_dirs,
	    mean_move_dir,
	    is_right_wing
	);
}

inline std::vector<wing_force_vec_3d>
gather_wing_forces_3d(const wing_forces_3d &forces) {
	std::vector<wing_force_vec_3d> result;
	result.reserve(
	    forces.sectional_lift.size() + forces.sectional_drag.size() + 1
	);

	for (const auto &lift : forces.sectional_lift) {
		result.push_back(lift);
	}
	for (const auto &drag : forces.sectional_drag) {
		result.push_back(drag);
	}
	result.push_back(forces.induced_drag);

	return result;
}
